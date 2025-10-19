savedcmd_dump_process.mod := printf '%s\n'   dump_process.o | awk '!x[$$0]++ { print("./"$$0) }' > dump_process.mod
