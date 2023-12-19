import commonHelpers = require('./helpers');
import path = require('path');
import tl = require('azure-pipelines-task-lib/task');

function run(): void {
    try {
        const AppAttachFrameworkDll = path.join(__dirname, 'lib', 'AppAttachFrameworkDLL');
        if (tl.exist(AppAttachFrameworkDll)) {
            tl.rmRF(AppAttachFrameworkDll);
        }
        commonHelpers.installNuget(commonHelpers.APPATTACH_FRAMEWORK_NUPKG_DIR, AppAttachFrameworkDll, 'net472');
	} catch (error) {
		console.error(tl.loc("AppAttachFramework Install Error", error));
	} 
}

run();