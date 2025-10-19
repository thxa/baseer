savedcmd_hello.mod := printf '%s\n'   hello.o | awk '!x[$$0]++ { print("./"$$0) }' > hello.mod
