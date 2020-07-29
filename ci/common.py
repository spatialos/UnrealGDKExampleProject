# coding=utf-8
import os
import re
import json
import subprocess

def log(msg, expand=False):
    if(expand):
        print("+++ %s" % msg)
    else:
        print("--- %s" % msg)


def run_command(cmd, args):
    os.system('%s %s' % (cmd, args))


def run_shell(cmds):
    cmdline = ' '.join(cmds)
    return subprocess.Popen(cmdline, shell=True, stdout=subprocess.PIPE,
                     stderr=subprocess.PIPE)


def get_buildkite_meta_data(name):
    cmds = [
        'buildkite-agent',
        'meta-data',
        'get',
        '"%s"' % name
    ]
    res = run_shell(cmds)
    return res.stdout.read()


def get_environment_variable(name, default_value):
    return os.environ.get(name) or default_value
