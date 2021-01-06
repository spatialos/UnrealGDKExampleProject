# coding=utf-8

# Purpose:
#   Generate auth token can be executed in any platform

import os
import sys
import json
import common
import random
import string
import datetime

def run_shell(cmds):
    res = common.run_shell(cmds)
    return res.stdout.read().decode('UTF-8').strip()


def set_gdk_commit_hash(repo, branch):
    cmds = ['git', 'ls-remote', '--head', repo, branch]
    commit_hash = run_shell(cmds)[0:6]
    common.set_buildkite_meta_data('gdk_commit_hash', commit_hash)
    return commit_hash


def random_string(length):
    return ''.join(random.choice(string.ascii_lowercase + string.digits) for _ in range(length))


def generate_deployment_name(commit_hash):
    now = datetime.datetime.now()
    time_string = now.strftime('%m%d_%H%M')
    engine_version_count = 1
    if not common.debug:
        engine_version_count = int(common.get_buildkite_meta_data('engine-version-count'))
    for i in range(0, engine_version_count):
        idx_string = '%d' % (i+1)
        deployment_name = 'epci%s_%s_%s_%s' % (idx_string, random_string(4), time_string, commit_hash)
        common.set_buildkite_meta_data('deployment-name-%s' % idx_string, deployment_name)


def generate_auth_token(project_name):
    with common.pushd('spatial') as ctx:
        cmds = ['spatial', 
        'project', 
        'auth', 
        'dev-auth-token', 
        'create', 
        '--description="Token generated for Example Project CI"',
        '--lifetime=24h',
        'project_name="%s"' % project_name,
        '--json_output'
        ]
        result = json.loads(run_shell(cmds))
        if 'json_data' in result:
            common.set_buildkite_meta_data('auth-token', result['json_data']['token_secret'])
        else:
            print(result)
            common.log('Unable to create a development authentication token. Please take a look at the logs.')
            exit(1)
            

if __name__ == "__main__":
    repo = common.get_environment_variable('GDK_REPOSITORY', 'git@github.com:spatialos/UnrealGDK.git')
    branch = common.get_environment_variable('GDK_BRANCH', 'master')
    project_name = common.get_environment_variable('SPATIAL_PROJECT_NAME', 'unreal_gdk')
    commit_hash = set_gdk_commit_hash(repo, branch)
    generate_deployment_name(commit_hash)
    generate_auth_token(project_name)
