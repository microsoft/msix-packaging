/*
 * Copyright (C) 2017 Microsoft.  All rights reserved.
 * See LICENSE file in the project root for full license information.
 */
package com.microsoft.msix;

import android.content.res.Resources;

public class JniHelper {

    static public String getLanguage() {
        // TODO: Change the signature of this method to return an array,
        //       Find out the API level of the system and make the corresponding
        //       call and use Configuration.getLocales() if possible.
        //       Configuration.locale was deprecated in API level 24, 
        //       but MSIX min API level is 19
        return Resources.getSystem().getConfiguration().locale.toString();
    }

}

