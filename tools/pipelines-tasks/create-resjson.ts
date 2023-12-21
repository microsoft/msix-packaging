// Adapted from github.com/microsoft/azure-pipelines-tasks/
import fs = require('fs');
import os = require('os');
import path = require('path');
import shell = require('shelljs');

const taskNames: string[] =
[
    'AppInstallerFile',
    'AVDAppAttachPublish',
    'MsixAppAttach',
    'MsixPackaging',
    'MsixSigning',
]

const shellAssert = () => {
    var errMsg = shell.error();
    if (errMsg) {
        throw new Error(errMsg);
    }
};

const mkdir = (options: string, target: string) => {
    if (target) {
        shell.mkdir(options, target);
    }
    else {
        shell.mkdir(options);
    }

    shellAssert();
}

const fileToJson = (file: string) => {
    var jsonFromFile = JSON.parse(fs.readFileSync(file).toString());
    return jsonFromFile;
}

const createResjson = (task: any, taskPath: string) => {
    const resources: any = {};
    if (task.hasOwnProperty('friendlyName')) {
        resources['loc.friendlyName'] = task.friendlyName;
    }

    if (task.hasOwnProperty('helpMarkDown')) {
        resources['loc.helpMarkDown'] = task.helpMarkDown;
    }

    if (task.hasOwnProperty('description')) {
        resources['loc.description'] = task.description;
    }

    if (task.hasOwnProperty('instanceNameFormat')) {
        resources['loc.instanceNameFormat'] = task.instanceNameFormat;
    }

    if (task.hasOwnProperty('releaseNotes')) {
        resources['loc.releaseNotes'] = task.releaseNotes;
    }

    if (task.hasOwnProperty('groups')) {
        task.groups.forEach(function (group: any) {
            if (group.hasOwnProperty('name')) {
                resources['loc.group.displayName.' + group.name] = group.displayName;
            }
        });
    }

    if (task.hasOwnProperty('inputs')) {
        task.inputs.forEach(function (input: any) {
            if (input.hasOwnProperty('name')) {
                resources['loc.input.label.' + input.name] = input.label;

                if (input.hasOwnProperty('helpMarkDown') && input.helpMarkDown) {
                    resources['loc.input.help.' + input.name] = input.helpMarkDown;
                }
            }
        });
    }

    if (task.hasOwnProperty('messages')) {
        Object.keys(task.messages).forEach(function (key: any) {
            resources['loc.messages.' + key] = task.messages[key];
        });
    }

    var resjsonPath = path.join(taskPath, 'Strings', 'resources.resjson', 'en-US', 'resources.resjson');
    mkdir('-p', path.dirname(resjsonPath));
    var resjsonContent = JSON.stringify(resources, null, /* space */ 2);
    if (process.platform == 'win32') {
        resjsonContent = resjsonContent.replace(/\n/g, os.EOL);
    }
    fs.writeFileSync(resjsonPath, resjsonContent);
};

const createTaskLocJson = (taskPath: string) => {
    const taskJsonPath = path.join(taskPath, 'task.json');
    const taskLoc: any = fileToJson(taskJsonPath);
    taskLoc.friendlyName = 'ms-resource:loc.friendlyName';
    taskLoc.helpMarkDown = 'ms-resource:loc.helpMarkDown';
    taskLoc.description = 'ms-resource:loc.description';
    taskLoc.instanceNameFormat = 'ms-resource:loc.instanceNameFormat';
    if (taskLoc.hasOwnProperty('releaseNotes')) {
        taskLoc.releaseNotes = 'ms-resource:loc.releaseNotes';
    }

    if (taskLoc.hasOwnProperty('groups')) {
        taskLoc.groups.forEach(function (group: any) {
            if (group.hasOwnProperty('name')) {
                group.displayName = 'ms-resource:loc.group.displayName.' + group.name;
            }
        });
    }

    if (taskLoc.hasOwnProperty('inputs')) {
        taskLoc.inputs.forEach(function (input: any) {
            if (input.hasOwnProperty('name')) {
                input.label = 'ms-resource:loc.input.label.' + input.name;

                if (input.hasOwnProperty('helpMarkDown') && input.helpMarkDown) {
                    input.helpMarkDown = 'ms-resource:loc.input.help.' + input.name;
                }
            }
        });
    }

    if (taskLoc.hasOwnProperty('messages')) {
        Object.keys(taskLoc.messages).forEach(function (key) {
            taskLoc.messages[key] = 'ms-resource:loc.messages.' + key;
        });
    }

    var taskLocContent = JSON.stringify(taskLoc, null, 2);
    if (process.platform == 'win32') {
        taskLocContent = taskLocContent.replace(/\n/g, os.EOL);
    }
    fs.writeFileSync(path.join(taskPath, 'task.loc.json'), taskLocContent);
};

for (const taskName of taskNames)
{
    const taskPath = path.join(__dirname, taskName);
    const taskJsonPath = path.join(taskPath, 'task.json');
    const taskJson = fileToJson(taskJsonPath);

    // create loc files
    createTaskLocJson(taskPath);
    createResjson(taskJson, taskPath);
}