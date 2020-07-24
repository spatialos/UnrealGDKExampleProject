# coding=utf-8

# Purpose:
#   test Android & iOS packages connection to SpatialOS
#   use GCS to upload to firebase directly will fail because of the file size being too large
#   test:
#       1. download Android & iOS packages to local
#       2. upload Android & iOS packages to gcloud
#       3. download the log file
#       4. analyze log file if spatialos connection successful keywords found

import io
import os
import re
import json
import sys
import common
import platform

def switch_gcloud_project(project_id):
    args = ['config', 'set', 'project', project_id]
    common.run_command('gcloud', ' '.join(args)) 


def check_firebase_log(app_platform, url, device, success_keyword):
    filename = ''
    localfilename = '%s.txt' % device
    if os.path.exists(localfilename):
        os.remove(localfilename)
    if app_platform == 'android':
        filename = 'logcat'
    elif app_platform == 'ios':
        filename = 'syslog.txt'
    else:
        print("unsupported platform:%s" % app_platform)
        return ''
    fullurl = 'gs://%s%s/%s' % (url, device, filename)
    common.run_command('gsutil', 'cp %s %s' % (fullurl, localfilename))

    if os.path.exists(localfilename):
        with io.open(localfilename, encoding="utf8") as fp:
            line = fp.readline()
            while line:                
                if success_keyword in line:
                    return True
                line = fp.readline()
    return False

def gcloud_upload(app_platform, app_path):
    cmds = [
        'gcloud',
        'beta',
        'firebase',
        'test',
        app_platform,
        'run',
        '--type game-loop',
        '--app %s' % app_path,
        '--scenario-numbers 1',
        '--format="json"',
    ]
    res = common.run_shell(cmds)
    gcloud_storage_url = ''
    gcloud_storage_keyword = common.get_environment_variable(
        'GCLOUD_STORAGE_KEYWORD', 'https://console.developers.google.com/storage/browser/')
    success_keyword = common.get_environment_variable(
        'SUCCESS_KEYWORD', 'PlayerSpawn returned from server sucessfully')
    for line in res.stderr.readlines():
        utf8 = line.decode('UTF-8').strip()
        if len(utf8) > 0:
            print('stderr:%s' % utf8)
        if gcloud_storage_keyword in utf8:
            url = re.findall(r'\[(.*?)\]', utf8)
            gcloud_storage_url = url[0][len(gcloud_storage_keyword):]
    total = 0
    succeeded = 0
    output = res.stdout.read().decode('UTF-8')
    print('upload package output:%s' % output)
    for result in json.loads(output):
        total += 1
        if check_firebase_log(app_platform, gcloud_storage_url, result['axis_value'], success_keyword):
            succeeded += 1
    return succeeded, total

def get_gcs_and_local_path(app_platform, engine_commit_formatted_hash):
    filename = ''
    localfilename = ''
    path = 'cooked-%s' % app_platform
    if app_platform == 'android':
        localfilename = 'GDKShooter-armv7-es2.apk'
        filename = '%s/Android_ASTC/%s' % (path, android_flavor, localfilename)
        agentplatform = 'windows'
    else:
        localfilename = 'GDKShooter.ipa'
        filename = '%s/IOS/%s' % (path,localfilename)
        agentplatform = 'macos'
    jobid = common.get_buildkite_meta_data('%s-build-%s-job-id' % (engine_commit_formatted_hash, app_platform))
    queueid = common.get_buildkite_meta_data('%s-build-%s-queue-id' % (engine_commit_formatted_hash, app_platform))
    organization = common.get_environment_variable('BUILDKITE_ORGANIZATION_SLUG','improbable')
    pipeline = common.get_environment_variable('BUILDKITE_PIPELINE_SLUG','unrealgdkexampleproject-nightly')
    buildid = common.get_environment_variable('BUILDKITE_BUILD_ID','')
    gcshead = 'gs://io-internal-infra-intci-artifacts-production'
    gcs_path = '%s/organizations/%s/pipelines/%s/builds/%s/jobs/%s/%s/%s/%s' % (gcshead, organization, pipeline, buildid, queueid, jobid, agentplatform, filename)
    return gcs_path,localfilename

def download_app(app_platform, engine_commit_formatted_hash):
    gclpath, localpath = get_gcs_and_local_path(app_platform, engine_commit_formatted_hash)
    if os.path.exists(localpath):
        os.remove(localpath)
    args = ['cp', gclpath, localpath]
    common.run_command('gsutil', ' '.join(args))
    return localpath

if __name__ == "__main__":
    app_platform = sys.argv[1]
    engine_commit_formatted_hash = sys.argv[2]
    parent_event = "automatic-test-%s-on-%s:" % (app_platform, platform.system())
    cmds = ['gcloud', 'config', 'get-value', 'project']
    res = common.run_shell(cmds)
    project = res.stdout.read().decode('UTF-8')
    
    # set gcloud project_id both Windows & Mac
    gcloud_project_id = common.get_environment_variable(
        'GCLOUD_PROJECT_ID', 'chlorodize-bipennated-8024348')

    # set to firebase gcloud project
    switch_gcloud_project(gcloud_project_id)

    # download app to local
    localpath = download_app(app_platform, engine_commit_formatted_hash)

    # upload local app to firebase for test
    succeed, total = gcloud_upload(app_platform, localpath)
    
    # set to buildkite infrastructure gcloud project
    switch_gcloud_project(project)

    # update firebase succeed/total value
    exit_value = 0 if succeed == total and succeed != 0 else 1
    exit(exit_value)
