# coding=utf-8
import os
import subprocess

debug = True


def log(msg, expand=False):
    if (expand):
        print("+++ %s" % msg)
    else:
        print("--- %s" % msg)


def run_command(cmd, args):
    os.system('%s %s' % (cmd, args))


def run_shell(cmds):
    cmdline = ' '.join(cmds)
    if debug:
        print(cmdline)
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
    return res.stdout.read().decode('UTF-8').strip()


def set_buildkite_meta_data(name, value):
    cmds = [
        'buildkite-agent',
        'meta-data',
        'set',
        '"%s"' % name,
        '"%s"' % value,
    ]
    res = run_shell(cmds)
    return res.stdout.read().decode('UTF-8').strip()


def get_environment_variable(name, default_value):
    return os.environ.get(name) or default_value


class PushdContext:
    cwd = None
    original_dir = None

    def __init__(self, dirname):
        self.cwd = os.path.realpath(dirname)

    def __enter__(self):
        self.original_dir = os.getcwd()
        os.chdir(self.cwd)
        return self

    def __exit__(self, type, value, tb):
        os.chdir(self.original_dir)

def pushd(dirname):
    return PushdContext(dirname)