# coding=utf-8

import sys
import json
import common
import requests
import collections


def slack_notify(channel, slack_webhook_url):
    project_name = common.get_environment_variable(
        'SPATIAL_PROJECT_NAME', 'unreal_gdk')
    mac_build = common.get_environment_variable('MAC_BUILD', 'false')
    firebase_test = common.get_environment_variable('FIREBASE_TEST', 'false')
    build_message = common.get_environment_variable(
        'BUILDKITE_MESSAGE', 'test build message')
    gdk_branch_name = common.get_environment_variable('GDK_BRANCH', 'master')
    buildkite_commit = common.get_environment_variable('BUILDKITE_COMMIT', '')
    # Set from generate-auto-token
    gdk_commit_hash = common.get_buildkite_meta_data('gdk_commit_hash')
    gdk_commit_url = 'https://github.com/spatialos/UnrealGDK/commit/%s' % gdk_commit_hash
    project_commit_url = 'https://github.com/spatialos/UnrealGDKExampleProject/commit/%s' % buildkite_commit
    nightly_build = common.get_environment_variable('NIGHTLY_BUILD', 'false')

    # Build Slack text
    slack_text = ''
    if nightly_build == 'true':
        slack_text = ':night_with_stars: Nightly build of *Example Project* succeeded.'
    elif firebase_test == 'true':
        slack_text = ':night_with_stars: Firebase Connection Tests for the *Example Project* succeeded.'
    else:
        build_creator = common.get_environment_variable(
            'BUILDKITE_BUILD_CREATOR', '')
        slack_text = '*Example Project* build by %s succeeded.' % build_creator
    build_message_len = min(64, len(build_message))
    formatted_build_message = build_message[0:build_message_len]
    build_url = common.get_environment_variable('BUILDKITE_BUILD_URL', '')
    json_message = {
        'text': slack_text,
        'channel': channel,
        'attachments': [
            {
                'fallback': 'Find build here: %s.' % build_url,
                'color': 'good',
                'fields': [
                    {
                        'title': 'Build Message',
                        'value': formatted_build_message,
                        'short': 'true'
                    },
                    {
                        'title': 'Example Project branch',
                        'value': common.get_environment_variable('BUILDKITE_BRANCH', ''),
                        'short': 'true'
                    },
                    {
                        'title': 'GDK branch',
                        'value': gdk_branch_name,
                        'short': 'true'
                    }
                ],
                'actions': [
                    {
                        'type': 'button',
                        'text': ':github: Project commit',
                        'url': project_commit_url,
                        'style': 'primary'
                    },
                    {
                        'type': 'button',
                        'text': ':github: GDK commit',
                        'url': gdk_commit_url,
                        'style': 'primary'
                    },
                    {
                        'type': 'button',
                        'text': ':buildkite: BK build',
                        'url': build_url,
                        'style': 'primary'
                    }
                ]
            }
        ]
    }
    if mac_build == 'true':
        ios_message = {
            'title': 'iOS Test Result',
            'value': 'succeeded',
            'short': 'true'
        }
        json_message['attachments'][0]['fields'].insert(0, ios_message)
    if firebase_test == 'true':
        android_message = {
            'title': 'Android Test Result',
            'value': 'succeeded',
            'short': 'true'
        }
        json_message['attachments'][0]['fields'].insert(0, android_message)
    launch_deployment = common.get_environment_variable(
        'START_DEPLOYMENT', 'true')
    
    # Set from generate-pipeline-steps
    engine_version_count = common.get_buildkite_meta_data(
        'engine-version-count')
    if launch_deployment == 'true':
        for i in range(0, int(engine_version_count)):
            index_str = '%d' % (i + 1)
            name = 'deployment-name-%s' % index_str

            # Set from generate-auto-token
            deployment_name = common.get_buildkite_meta_data(name)
            deployment_url = 'https://console.improbable.io/projects/%s/deployments/%s/overview' % (
                project_name, deployment_name)
            deployment_button = {
                'type': 'button',
                'text': ':cloud: Deployment %s' % index_str,
                'url': deployment_url,
                'style': 'primary'
            }
            json_message['attachments'][0]['actions'].append(deployment_button)
    headers = {'Content-Type': 'application/json', 'Accept': 'text/plain'}
    res = requests.post(slack_webhook_url, data=json.dumps(
        json_message), headers=headers)
    return res.text == 'ok'


if __name__ == '__main__':    
    common.log('generate-slack-webhook-url')
    cmds = [
        'imp-ci',
        'secrets',
        'read',
        '--environment=production',
        '--buildkite-org=improbable',
        '--secret-type=slack-webhook',
        '--secret-name=unreal-gdk-slack-web-hook'
    ]
    res = common.run_shell(cmds)
    for line in res.stderr.readlines():
        utf8 = line.decode('UTF-8').strip()
        if len(utf8) > 0:
            print('%s' % utf8)
    output = res.stdout.read().decode('UTF-8')
    slack_webhook_url = json.loads(output)['url']

    common.log('slack-notify')
    slack_channel = common.get_environment_variable(
        'SLACK_CHANNEL', '#unreal-gdk-builds')
    result = slack_notify(slack_channel, slack_webhook_url)

    exit_value = 0 if result == True else 1
    exit(exit_value)
