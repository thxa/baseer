import { spawn } from "node:child_process";
import { randomUUID } from "node:crypto";
import path from "node:path";

// ── Types ──────────────────────────────────────────────────────────────────────

interface ClaudeResult {
  type: string;
  subtype: string;
  result: string;
  session_id: string;
  total_cost_usd: number;
  num_turns: number;
  is_error: boolean;
}

// ── Config ─────────────────────────────────────────────────────────────────────

const CWD = process.cwd();
const WORKTREE_NAME = `pipeline-${randomUUID().slice(0, 8)}`;

// ── Helpers ────────────────────────────────────────────────────────────────────

function log(phase: string, msg: string) {
  const dim = "\x1b[2m";
  const cyan = "\x1b[36m";
  const reset = "\x1b[0m";
  const bold = "\x1b[1m";
  console.log(`${dim}──${reset} ${cyan}${bold}[${phase}]${reset} ${msg}`);
}

function logCost(result: ClaudeResult) {
  const dim = "\x1b[2m";
  const reset = "\x1b[0m";
  console.log(
    `   ${dim}turns: ${result.num_turns} | cost: $${result.total_cost_usd.toFixed(4)}${reset}`
  );
}

function fail(msg: string): never {
  console.error(`\x1b[31mError:\x1b[0m ${msg}`);
  process.exit(1);
}

/**
 * Runs `claude` CLI in non-interactive mode and returns the parsed JSON result.
 * Streams stderr (progress) to the terminal in real time.
 */
function runClaude(args: string[], cwd: string = CWD): Promise<ClaudeResult> {
  return new Promise((resolve, reject) => {
    const proc = spawn("claude", args, {
      cwd,
      stdio: ["ignore", "pipe", "inherit"],
      env: { ...process.env },
    });

    let stdout = "";

    proc.stdout.on("data", (chunk: Buffer) => {
      stdout += chunk.toString();
    });

    proc.on("error", (err) => reject(new Error(`Failed to spawn claude: ${err.message}`)));

    proc.on("close", (code) => {
      if (code !== 0) {
        reject(new Error(`claude exited with code ${code}\n${stdout}`));
        return;
      }
      try {
        const result = JSON.parse(stdout) as ClaudeResult;
        if (result.is_error) {
          reject(new Error(`Claude returned an error: ${result.result}`));
          return;
        }
        resolve(result);
      } catch {
        reject(new Error(`Failed to parse claude output:\n${stdout.slice(0, 500)}`));
      }
    });
  });
}

// ── Pipeline Steps ─────────────────────────────────────────────────────────────

async function createWorktree(): Promise<string> {
  log("Worktree", `Creating worktree: ${WORKTREE_NAME}`);

  const worktreePath = path.resolve(CWD, "..", `baseer-${WORKTREE_NAME}`);
  const branch = WORKTREE_NAME;

  const proc = spawn("git", ["worktree", "add", "-b", branch, worktreePath], {
    cwd: CWD,
    stdio: "inherit",
  });

  await new Promise<void>((resolve, reject) => {
    proc.on("error", reject);
    proc.on("close", (code) =>
      code === 0 ? resolve() : reject(new Error(`git worktree add failed (code ${code})`))
    );
  });

  log("Worktree", `Created at ${worktreePath} (branch: ${branch})`);
  return worktreePath;
}

async function plan(prompt: string, worktreePath: string): Promise<ClaudeResult> {
  log("Plan", "Generating implementation plan...");

  const result = await runClaude([
    "--print",
    "--output-format", "json",
    "--permission-mode", "plan",
    "--dangerously-skip-permissions",
    prompt,
  ], worktreePath);

  log("Plan", "Plan created successfully.");
  logCost(result);
  return result;
}

async function implement(sessionId: string, worktreePath: string): Promise<ClaudeResult> {
  log("Implement", "Implementing the plan...");

  const result = await runClaude([
    "--print",
    "--output-format", "json",
    "--resume", sessionId,
    "--permission-mode", "acceptEdits",
    "--dangerously-skip-permissions",
    "Exit plan mode. Now implement the plan you created. Do the work step by step.",
  ], worktreePath);

  log("Implement", "Implementation complete.");
  logCost(result);
  return result;
}

async function simplify(sessionId: string, worktreePath: string): Promise<ClaudeResult> {
  log("Simplify", "Running /simplify to clean up the code...");

  const result = await runClaude([
    "--print",
    "--output-format", "json",
    "--resume", sessionId,
    "--permission-mode", "acceptEdits",
    "--dangerously-skip-permissions",
    "/simplify",
  ], worktreePath);

  log("Simplify", "Simplification complete.");
  logCost(result);
  return result;
}

async function commit(sessionId: string, worktreePath: string): Promise<ClaudeResult> {
  log("Commit", "Committing changes...");

  const result = await runClaude([
    "--print",
    "--output-format", "json",
    "--resume", sessionId,
    "--permission-mode", "acceptEdits",
    "--dangerously-skip-permissions",
    "/commit",
  ], worktreePath);

  log("Commit", "Changes committed.");
  logCost(result);
  return result;
}

async function openPR(sessionId: string, worktreePath: string): Promise<ClaudeResult> {
  log("PR", "Opening pull request...");

  const result = await runClaude([
    "--print",
    "--output-format", "json",
    "--resume", sessionId,
    "--permission-mode", "acceptEdits",
    "--dangerously-skip-permissions",
    "Create a pull request for these changes to main. Use `gh pr create` with a clear title and description summarizing what was done.",
  ], worktreePath);

  log("PR", "Pull request created.");
  logCost(result);
  return result;
}

async function cleanupWorktree(worktreePath: string) {
  const dim = "\x1b[2m";
  const reset = "\x1b[0m";
  console.log(`\n${dim}Tip: when done, clean up the worktree with:${reset}`);
  console.log(`  git worktree remove ${worktreePath}`);
}

// ── Main ───────────────────────────────────────────────────────────────────────

async function main() {
  const prompt = process.argv.slice(2).join(" ").trim();

  if (!prompt) {
    fail("Usage: bun pipeline 'YOUR PROMPT HERE'");
  }

  console.log("\n\x1b[1m🔧 Baseer Production Pipeline\x1b[0m\n");
  console.log(`Prompt: ${prompt}\n`);

  let totalCost = 0;
  const trackCost = (r: ClaudeResult) => {
    totalCost += r.total_cost_usd;
    return r;
  };

  // Step 1: Create worktree
  const worktreePath = await createWorktree();

  try {
    // Step 2: Plan
    const planResult = trackCost(await plan(prompt, worktreePath));

    // Step 3: Implement
    const implResult = trackCost(await implement(planResult.session_id, worktreePath));

    // Step 4: Simplify
    const simpResult = trackCost(await simplify(implResult.session_id, worktreePath));

    // Step 5: Commit
    const commitResult = trackCost(await commit(simpResult.session_id, worktreePath));

    // Step 6: Open PR
    const prResult = trackCost(await openPR(commitResult.session_id, worktreePath));

    // Summary
    console.log("\n\x1b[32m\x1b[1m✓ Pipeline complete!\x1b[0m");
    console.log(`  Total cost: $${totalCost.toFixed(4)}`);
    console.log(`  PR result: ${prResult.result.slice(0, 200)}`);

    await cleanupWorktree(worktreePath);
  } catch (err) {
    console.error(`\n\x1b[31mPipeline failed:\x1b[0m`, (err as Error).message);
    await cleanupWorktree(worktreePath);
    process.exit(1);
  }
}

main();
