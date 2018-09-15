/*
 * Copyright (C) 2017 Microsoft.  All rights reserved.
 * See LICENSE file in the project root for full license information.
 */
package com.microsoft.msix;

import android.content.res.Resources;
import android.os.LocaleList;

import java.util.ArrayList;
import java.util.List;

public class Language {

    static public String[] getLanguages() {
        List<String> languageList = new ArrayList<String>();
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
            LocaleList ll = Resources.getSystem().getConfiguration().getLocales();
            for(int i = 0; i < ll.size(); i++) {
                languageList.add(ll.get(i).toString());
            }
        } else {
            languageList.add(Resources.getSystem().getConfiguration().locale.toString());
        }
        return languageList.toArray(new String[0]);
    }

}
