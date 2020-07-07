# coding=utf-8
import os
import re
import json
import platform
import subprocess

debug = False


def log(msg, expand=False):
    if(expand):
        print("+++ %s" % msg)
    else:
        print("--- %s" % msg)


def buildkite_project_name():
    return "build-unreal-gdk-example-project-:%s:" % platform.system()


def run_command(cmd, args):
    print('cmd:%s' % cmd)
    print('args:%s' % args)
    os.system('%s %s' % (cmd, args))


def run_shell(cmds):
    s = ' '.join(cmds)
    print(s)
    return subprocess.Popen(s, shell=True, stdout=subprocess.PIPE,
                     stderr=subprocess.PIPE)


def start_event(name):
    args = [
        'events',
        'new',
        '--name',
        name,
        '--child-of',
        buildkite_project_name()
    ]
    s = ' '.join(args)
    if not debug:
        run_command("imp-ci", s)
    else:
        log("start imp-ci args:%s" % s)
    log(name, False)


def finish_event(name):
    args = [
        'events',
        'new',
        '--name',
        name,
        '--child-of',
        buildkite_project_name()
    ]
    s = ' '.join(args)
    if not debug:
        run_command("imp-ci", s)
    else:
        log("finish imp-ci args:%s" % args)


def set_buildkite_meta_data(name, value):
    cmd = 'buildkite-agent meta-data set "%s" "%s"' % (name, value)
    print(cmd)
    if not debug:
        os.system(cmd)


def get_buildkite_meta_data(name):
    cmds = [
        'buildkite-agent',
        'meta-data',
        'get',
        '"%s"' % name
    ]
    if debug:
        return ''
    else:
        res = run_shell(cmds)
        return res.stdout.read()


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
