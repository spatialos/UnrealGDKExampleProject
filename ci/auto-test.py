# coding=utf-8

# Purpose:
#   test Android & iOS packages connection to SpatialOS
#   use gcs to upload to firebase directly will be failed because of the file size is too large
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



def run_command(cmds):
    try:
        res = common.run_shell(cmds)
        for line in res.stderr.readlines():
            utf8 = line.decode('UTF-8')
            print(utf8)
        for line in res.stdout.readlines():
            utf8 = line.decode('UTF-8')
            print(utf8)
    except Exception as e:
        print(e)
        return 0, 1

# prepare gcloud env because of:
# 1. on windows agent there are no beta component of gcloud
# 2. there are no project_id setted both on windows and mac
def switch_gcloud_project():
    event_name = "switch_gcloud_project"
    common.start_event(event_name)
    
    # set gcloud project_id both Windows & Mac
    gcloud_project_id = common.get_environment_variable(
        'GCLOUD_PROJECT_ID', 'chlorodize-bipennated-8024348')
        
    cmds = [
        'gcloud',
        'config',
        'set',
        'project',
        gcloud_project_id
    ]
    run_command(cmds)
 
    common.finish_event(event_name)

def check_firebase_log(app_platform, url, device):
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

    success_keyword = common.get_environment_variable(
        'SUCCESS_KEYWORD', 'PlayerSpawn returned from server sucessfully')
    result = False
    if os.path.exists(localfilename):
        with io.open(localfilename, encoding="utf8") as fp:#
            line = fp.readline()
            while line:
                if success_keyword in line:
                    result = True
                    break
                line = fp.readline()
    return result


def gcloud_upload(app_platform, app_path):
    event_name = "gcloud_upload"
    common.start_event(event_name)
    try:
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
        for line in res.stderr.readlines():
            utf8 = line.decode('UTF-8')
            print(utf8)
            if gcloud_storage_keyword in utf8:
                url = re.findall(r'\[(.*?)\]', utf8)
                gcloud_storage_url = url[0][len(gcloud_storage_keyword):]
        total = 0
        succeed = 0
        output = res.stdout.read().decode('UTF-8')
        print('upload package output:%s' % output)
        for i in json.loads(output):
            total += 1
            if check_firebase_log(app_platform, gcloud_storage_url, i['axis_value']):
                succeed += 1
        return succeed, total
    except Exception as e:
        print(e)
        return 0, 1
    finally:
        common.finish_event(event_name)

def get_gcs_and_local_path(app_platform):
    filename = ''
    localfilename = ''
    if app_platform == 'android':
        android_flavor = common.get_buildkite_meta_data('android-flavor')
        localfilename = common.get_environment_variable('ANDROID_FILENAME','GDKShooter-armv7-es2.apk')
        filename = 'cooked-android/Android_%s/%s' % (android_flavor, localfilename)
    else:
        localfilename = common.get_environment_variable('IOS_FILENAME','GDKShooter.ipa')
        filename = 'IOS/%s' % (localfilename)
    jobid = common.get_buildkite_meta_data('build-%s-job-id' % app_platform)
    organization = common.get_environment_variable('BUILDKITE_ORGANIZATION_SLUG','improbable')
    pipeline = common.get_environment_variable('BUILDKITE_PIPELINE_SLUG','unrealgdkexampleproject-nightly')
    buildid = common.get_environment_variable('BUILDKITE_BUILD_ID','')
    metadata = common.get_environment_variable('BUILDKITE_AGENT_META_DATA_QUEUE','')
    metadataos = common.get_environment_variable('BUILDKITE_AGENT_META_DATA_PLATFORM','')
    gcshead = 'gs://io-internal-infra-intci-artifacts-production'
    gcs_path = '%s/organizations/%s/pipelines/%s/builds/%s/jobs/%s/%s/%s/%s' % (gcshead, organization, pipeline, buildid, metadata, jobid, metadataos, filename)
    return gcs_path,localfilename

def download_app(app_platform):
    event_name = "download_app"
    common.start_event(event_name)
    gclpath, localpath = get_gcs_and_local_path(app_platform)
    if os.path.exists(localpath):
        os.remove(localpath)
    cmds = [
        'gsutil',
        'cp',
        gclpath,
        localpath
    ]
    try:
        res = common.run_shell(cmds)
        for line in res.stderr.readlines():
            utf8 = line.decode('UTF-8')
            print(utf8)
        for line in res.stdout.readlines():
            utf8 = line.decode('UTF-8')
            print(utf8)
    except Exception as e:
        print(e)

    common.finish_event(event_name)
    return localpath

if __name__ == "__main__":
    switch_gcloud_project()
    app_platform = sys.argv[1]
    localpath = download_app(app_platform)
    succeed, total = gcloud_upload(app_platform, localpath)
    print('succeed=%d total=%d' % (succeed, total))
    common.set_buildkite_meta_data('firebase-%s-succeed' % app_platform, '%d' % succeed)
    common.set_buildkite_meta_data('firebase-%s-total' % app_platform, '%d' % total)

