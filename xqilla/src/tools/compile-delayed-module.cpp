/*
 * Copyright (c) 2004-2009
 *     Oracle. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>

#include <xqilla/xqilla-simple.hpp>
#include <xqilla/debug/InteractiveDebugger.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

using namespace std;

void usage(const char *progname)
{
  const char *name = progname;
  while(*progname != 0) {
    if(*progname == '/' || *progname == '\\') {
      ++progname;
      name = progname;
    } else {
      ++progname;
    }
  }

  cerr << "Usage: " << name << " [options] <XQuery module>" << endl << endl;
  cerr << "-h                : Show this display" << endl;
}

namespace CompileDelayedModule {

void compile(const XMLCh *queryFile)
{
  XQilla::compileDelayedModule(queryFile);
}

}

int main(int argc, char *argv[])
{
  string filename;

  for(int i = 1; i < argc; ++i) {
    if(*argv[i] == '-' && argv[i][2] == '\0' ){

      // -h option, print usage
      if(argv[i][1] == 'h') {
        usage(argv[0]);
        return 0;
      }
      else {
        usage(argv[0]);
        return 1;
      }
    }
    else if(filename == "") {
      filename = argv[i];
    }
    else {
      usage(argv[0]);
      return 1;
    }
  }

  if(filename == "") {
    usage(argv[0]);
    return 1;
  }

  XQilla xqilla;
  try {
    CompileDelayedModule::compile(X(filename.c_str()));
  }
  catch(XQException &e) {
    cerr << UTF8(e.getXQueryFile()) << ":" << e.getXQueryLine() << ":" << e.getXQueryColumn()
         << ": error: " << UTF8(e.getError()) << endl;
    BaseInteractiveDebugger::outputLocation(e.getXQueryFile(), e.getXQueryLine(), e.getXQueryColumn());
    return 1;
  }

  return 0;
}
