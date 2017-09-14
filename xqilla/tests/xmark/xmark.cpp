/*
 * Copyright (c) 2001-2008
 *     DecisionSoft Limited. All rights reserved.
 * Copyright (c) 2004-2008
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
 *
 * $Id$
 */

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#include <time.h>
#define snprintf _snprintf
#else
#include <sys/time.h>
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <xqilla/xqilla-simple.hpp>
#include <xqilla/context/VariableStore.hpp>
#include <xqilla/context/ModuleResolver.hpp>
#include <xqilla/context/URIResolver.hpp>
#include <xqilla/utils/XQillaPlatformUtils.hpp>
#include <xqilla/xerces/XercesConfiguration.hpp>
#include <xqilla/fastxdm/FastXDMConfiguration.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif
using namespace std;

#define MAXIMUM_TIME_FOR_QUERIES 0.2
#define MILLISECS_IN_SECS 1000

Sequence query(XQilla &xqilla, DynamicContext *context, const Item::Ptr &ci, string query)
{
  AutoDelete<XQQuery> pquery(xqilla.parse(X(query.c_str()), context, 0, XQilla::NO_ADOPT_CONTEXT));
  context->setContextItem(ci);
  return pquery->execute(context)->toSequence(context);
}

string timestamp(string &dateTime)
{
  time_t tt;
  time(&tt);

  struct tm *tm_p;
#ifdef _MSC_VER   
  tm_p = localtime(&tt);
#else
  struct tm tm;
  tm_p = &tm;
  localtime_r(&tt, &tm);
#endif

  char szDate[256];
  sprintf(szDate,"%04d-%02d-%02dT%02d:%02d:%02dZ",
    tm_p->tm_year+1900,
    tm_p->tm_mon+1,
    tm_p->tm_mday,
    tm_p->tm_hour,
    tm_p->tm_min,
    tm_p->tm_sec);
  dateTime = szDate;

  sprintf(szDate,"%04d%02d%02d%02d%02d%02d",
    tm_p->tm_year+1900,
    tm_p->tm_mon+1,
    tm_p->tm_mday,
    tm_p->tm_hour,
    tm_p->tm_min,
    tm_p->tm_sec);
  return szDate;
}

class Timer
{
public:
  Timer() { reset(); }

  void reset()
  {
    start_ = 0;
    duration_ = 0;
  }

  void start()
  {
    start_ = getTime();
  }

  void stop()
  {
    unsigned long end = getTime();
    duration_ += end - start_;
  }

  double durationInSeconds() const
  {
    return ((double)duration_ / MILLISECS_IN_SECS);
  }

private:
  static unsigned long getTime()
  {
    return XMLPlatformUtils::getCurrentMillis();
  }

  unsigned long start_;
  unsigned long duration_;
};

class Stats {
public:
  string name;
  string data;
  string size;

  map<string, string> info;

  int count;
  Timer timer;

  Stats() {
    name.clear();
    data.clear();
    size.clear();
    reset();
  }

  void reset() {
    info.clear();
    count = 0;
    timer.reset();
  }
};

class StatsReporter {
public:
  StatsReporter(string name, bool verbose)
    : verbose_(verbose),
      totalTime_(0),
      textCentricTime_(0),
      dataCentricTime_(0),
      multipleDocumentTime_(0),
      singleDocumentTime_(0),
      file_((name + timestamp(timestamp_) + ".xml").c_str())
  {
    if(!file_.is_open()) {
      cerr << "Unable to open statistics file: " << name << timestamp_ << ".xml" << endl;
      exit(-1);
    }

    file_ << "<statistics";
    file_ << " timestamp=\"" << timestamp_ << "\"";
    file_ << ">" << endl;

    if(verbose_) {
      cout << "*********************************************************************" << endl;
      cout << "Benchmark Statistics" << endl << endl;
      ::snprintf(buffer_, 255, " %5s | %5s | %9s | %10s | %5s | %10s ",
        "Name", "Data", "Size", "Time/s", "Count", "Ops/s");
      cout << buffer_ << endl;
      cout << "-------+-------+-----------+------------+-------+------------" << endl;
    }
  }

  ~StatsReporter() {
    file_ << "</statistics>" << endl;
    file_.close();

    if(verbose_) {
      cout << endl;
      cout << "Text Centric Total time/s: " << textCentricTime_ << endl;
      cout << "Data Centric Total time/s: " << dataCentricTime_ << endl;
      cout << endl;
      cout << "Multiple Document Total time/s: " << multipleDocumentTime_ << endl;
      cout << "Single Document Total time/s: " << singleDocumentTime_ << endl;
      cout << endl;
      cout << "Total time/s: " << totalTime_ << endl;
      cout << "*********************************************************************" << endl;
    }
  }

  void reportStats(const Stats &stats) {
    file_ << "  <entry";
    file_ << " name=\"" << stats.name << "\"";
    file_ << ">" << endl;

    file_ << "    <data_type>" << stats.data << "</data_type>" << endl;
    file_ << "    <size>" << stats.size << "</size>" << endl;

    for(map<string, string>::const_iterator i = stats.info.begin();
        i != stats.info.end(); ++i) {
      file_ << "    <" << i->first << ">" << i->second << "</" << i->first << ">" << endl;
    }

    file_ << "    <count>" << stats.count << "</count>" << endl;
    file_ << "    <time>" << stats.timer.durationInSeconds() << "</time>" << endl;

    file_ << "  </entry>" << endl;

    if(verbose_) {
      ::snprintf(buffer_, 255, " %5s | %5s | %9s | %10f | %5i | %10f ",
        stats.name.c_str(),
        stats.data.c_str(),
        stats.size.c_str(),
        stats.timer.durationInSeconds(),
        stats.count,
        ((double)stats.count) / stats.timer.durationInSeconds());
        
      cout << buffer_ << endl;

      totalTime_ += stats.timer.durationInSeconds();
      if(stats.data.find("TC") != string::npos) {
        textCentricTime_ += stats.timer.durationInSeconds();
      }
      else {
        dataCentricTime_ += stats.timer.durationInSeconds();
      }
      if(stats.data.find("MD") != string::npos) {
        multipleDocumentTime_ += stats.timer.durationInSeconds();
      }
      else {
        singleDocumentTime_ += stats.timer.durationInSeconds();
      }
    }
  }

private:
  bool verbose_;
  char buffer_[255];
  double totalTime_;
  double textCentricTime_;
  double dataCentricTime_;
  double multipleDocumentTime_;
  double singleDocumentTime_;

  string timestamp_;
  ofstream file_;
};

class XMarkResolver : public URIResolver
{
public: 
  vector<string> defaultCollection;

  virtual bool resolveDocument(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection)
  {
    return false;
  }

  virtual bool resolveCollection(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection)
  {
    return false;
  }

  virtual bool resolveDefaultCollection(Sequence &result, DynamicContext* context, const QueryPathNode *projection)
  {
    for(vector<string>::iterator s = defaultCollection.begin(); s != defaultCollection.end(); ++s) {
      result.joinSequence(context->resolveDocument(X(s->c_str()), 0, projection));
    }
    return true;
  }
  virtual bool putDocument(const Node::Ptr &document, const XMLCh *uri, DynamicContext *context)
  {
    return false;
  }
 
};

void queryBenchmarkData(XQilla &xqilla, DynamicContext *config_context, const Item::Ptr &config, string &dataPath,
                        XQillaConfiguration *conf, Stats &stats, StatsReporter &reporter)
{
  // Find the files
  XMarkResolver resolver;
  Result files = query(xqilla, config_context, config, "for $a in /benchmark_data/data_type[@name = '" + stats.data
    + "']/size[@name = '" + stats.size + "']/file/@name return data($a)");
  Item::Ptr file;
  while((file = files->next(config_context)).notNull()) {
    string fullpath = dataPath + stats.data + "/" + stats.size + "/" + UTF8(file->asString(config_context));
    resolver.defaultCollection.push_back(fullpath);
  }

  // Find the queries
  Result queries = query(xqilla, config_context, config, "for $a at $pos in /benchmark_data/data_type[@name = '" + stats.data
    + "']/query return (data($a/description), data($a/action), $pos)");

  Item::Ptr query;
  while((query = queries->next(config_context)).notNull()) {
    stats.reset();

    stats.info["description"] = UTF8(query->asString(config_context));
    query = queries->next(config_context);
    string action = UTF8(query->asString(config_context));
    query = queries->next(config_context);
    stats.info["query_index"] = UTF8(query->asString(config_context));

    // Replace "input()" with the correct "collection()" function call
    string::size_type pos = action.find("input()");
    while(pos != string::npos) {
      action = action.replace(pos, 7, "collection()");
      pos = action.find("input()");
    }
    stats.info["action"] = "<![CDATA[" + action + "]]>";

    try {
      AutoDelete<DynamicContext> context(xqilla.createContext(XQilla::XQUERY, conf));
      context->registerURIResolver(&resolver, /*adopt*/false);

      AutoDelete<XQQuery> query(xqilla.parse(X(action.c_str()), context, 0, XQilla::NO_ADOPT_CONTEXT));

      // Run the query for real, until the total duration is at least MAXIMUM_TIME_FOR_QUERIES
      while(stats.timer.durationInSeconds() < MAXIMUM_TIME_FOR_QUERIES) {
        stats.timer.start();

        Result result = query->execute(context.get());
        Item::Ptr item;
        while((item = result->next(context.get())).notNull()) {
        }

        stats.timer.stop();
        ++stats.count;
      }
    }
    catch(XQException& e) {
      cerr << UTF8(e.getError()) << endl;
      cerr << "at " << UTF8(e.getXQueryFile()) << ":" << e.getXQueryLine() << ":" << e.getXQueryColumn() << endl;
      cerr << "at " << e.getCppFile() << ":" << e.getCppLine() << endl;
      exit(-1);
    }
    catch(DOMException &de) {
      cerr << "DOMException: " << UTF8(de.getMessage()) << endl;
      exit(-1);
    }
    catch(...) {
      cerr << "[Unknown exception]";
      exit(-1);
    }

    stats.name = "query";
    reporter.reportStats(stats);
  }
}

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

  cerr << "Usage: " << name << " [options] benchmark_config_file" << endl << endl;
  cerr << "-h             : Show this display" << endl;
  cerr << "-x             : Use the Xerces-C data model (default is the FastXDM)" << endl;
}

int main(int argc, char *argv[])
{
  string configPath;
  string dataPath;

  XercesConfiguration xercesConf;
  FastXDMConfiguration fastConf;
  XQillaConfiguration *conf = &fastConf;

  for(int i = 1; i < argc; ++i) {
    if(*argv[i] == '-' && argv[i][2] == '\0' ){

      switch(argv[i][1]) {
      case 'h': {
        usage(argv[0]);
        return 0;
      }
      case 'x': {
        conf = &xercesConf;
        break;
      }
      default: {
        cerr << "Unknown option: " << argv[i] << endl;
        usage(argv[0]);
        return 1;
      }
      }
    }
    else if(configPath.empty()) {
      configPath = argv[i];

      string::size_type pos = configPath.rfind("/");
      if(pos != string::npos) {
        dataPath = configPath.substr(0, pos + 1);
      }
    }
    else {
      cerr << "Too many parameters" << endl;
      usage(argv[0]);
      return 1;
    }
  }

  if(configPath.empty()) {
    cerr << "Path to the benchmark configuration not specified"<< endl;
    usage(argv[0]);
    return 1;
  }

  XQilla xqilla;

  AutoDelete<DynamicContext> context(xqilla.createContext());
  Sequence seq = context->resolveDocument(X(configPath.c_str()));
  if(seq.isEmpty()) {
    cerr << "Benchmark configuration not found"<< endl;
    usage(argv[0]);
    return 1;
  }
  Item::Ptr config = seq.first();

  Sequence data_types = query(xqilla, context, config, "distinct-values(/benchmark_data/data_type/@name)");
  Sequence size_types = query(xqilla, context, config, "distinct-values(/benchmark_data/data_type/size/@name)");

  StatsReporter reporter("statistics", /*verbose*/true);

  Stats stats;
  Item::Ptr data;
  Result data_result = data_types;
  while((data = data_result->next(context)).notNull()) {
    stats.data = UTF8(data->asString(context));

    Item::Ptr size;
    Result size_result = size_types;
    while((size = size_result->next(context)).notNull()) {
      stats.size = UTF8(size->asString(context));
      queryBenchmarkData(xqilla, context, config, dataPath, conf, stats, reporter);
    }
  }

  return 0;
}

