Import("env")
import os

def print_flash_args(*args, **kwargs):
    flash_command = env.subst("$UPLOADERFLAGS")
    print("Generated Flash Arguments:")
    print(flash_command)

env.AddPreAction("upload", print_flash_args)