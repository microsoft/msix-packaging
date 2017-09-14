/*
 * Copyright (c) 2001, 2008,
 *     DecisionSoft Limited. All rights reserved.
 * Copyright (c) 2004, 2015 Oracle and/or its affiliates. All rights reserved.
 *     
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

#ifndef _FTDISTANCE_HPP
#define _FTDISTANCE_HPP

#include <xqilla/fulltext/FTOption.hpp>
#include <xqilla/fulltext/FTRange.hpp>

class XQILLA_API FTDistance : public FTOption
{
public:
  FTDistance(const FTRange &range, FTUnit unit, XPath2MemoryManager *memMgr)
    : FTOption(FTSelection::DISTANCE, memMgr), arg_(0), range_(range), unit_(unit) {}
  FTDistance(const FTRange &range, FTUnit unit, FTSelection *arg, XPath2MemoryManager *memMgr)
    : FTOption(FTSelection::DISTANCE, memMgr), arg_(arg), range_(range), unit_(unit) {}

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  virtual void setArgument(FTSelection *arg) { arg_ = arg; }
  FTSelection *getArgument() const { return arg_; }
  const FTRange &getRange() const { return range_; }
  FTUnit getUnit() const { return unit_; }

private:
  FTSelection *arg_;
  FTRange range_;
  FTUnit unit_;
};

class XQILLA_API FTDistanceLiteral : public FTOption
{
public:
  FTDistanceLiteral(FTRange::Type type, unsigned int distance, unsigned int distance2,
                    FTUnit unit, XPath2MemoryManager *memMgr)
    : FTOption(DISTANCE_LITERAL, memMgr), arg_(0), type_(type), distance_(distance),
      distance2_(distance2), unit_(unit) {}
  FTDistanceLiteral(FTSelection *arg, FTRange::Type type, unsigned int distance,
                    unsigned int distance2, FTUnit unit, XPath2MemoryManager *memMgr)
    : FTOption(DISTANCE_LITERAL, memMgr), arg_(arg), type_(type), distance_(distance),
      distance2_(distance2), unit_(unit) {}

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  virtual void setArgument(FTSelection *arg) { arg_ = arg; }
  FTSelection *getArgument() const { return arg_; }
  const FTRange::Type getType() const { return type_; }
  unsigned int getDistance() const { return distance_; }
  unsigned int getDistance2() const { return distance2_; }
  FTUnit getUnit() const { return unit_; }

private:
  FTSelection *arg_;
  FTRange::Type type_;
  unsigned int distance_, distance2_;
  FTUnit unit_;
};

class FTDistanceMatches : public AllMatches
{
public:
  FTDistanceMatches(const LocationInfo *info, unsigned int distance, FTOption::FTUnit unit, AllMatches *arg)
    : AllMatches(info), distance_(distance), unit_(unit), arg_(arg), excludes_() {}
  ~FTDistanceMatches();
  bool next(DynamicContext *context);
  const StringMatches &getStringIncludes();
  const StringMatches &getStringExcludes();

  virtual bool distanceMatches(unsigned int actual) const = 0;

protected:
  unsigned int distance_;
  FTOption::FTUnit unit_;
  AllMatches *arg_;
  StringMatches excludes_;
};

class FTDistanceExactlyMatches : public FTDistanceMatches
{
public:
  FTDistanceExactlyMatches(const LocationInfo *info, unsigned int distance, FTOption::FTUnit unit, AllMatches *arg)
    : FTDistanceMatches(info, distance, unit, arg) {}
  bool distanceMatches(unsigned int actual) const;
};

class FTDistanceAtLeastMatches : public FTDistanceMatches
{
public:
  FTDistanceAtLeastMatches(const LocationInfo *info, unsigned int distance, FTOption::FTUnit unit, AllMatches *arg)
    : FTDistanceMatches(info, distance, unit, arg) {}
  bool distanceMatches(unsigned int actual) const;
};

class FTDistanceAtMostMatches : public FTDistanceMatches
{
public:
  FTDistanceAtMostMatches(const LocationInfo *info, unsigned int distance, FTOption::FTUnit unit, AllMatches *arg)
    : FTDistanceMatches(info, distance, unit, arg) {}
  bool distanceMatches(unsigned int actual) const;
};

class FTDistanceFromToMatches : public FTDistanceMatches
{
public:
  FTDistanceFromToMatches(const LocationInfo *info, unsigned int from, unsigned int to, FTOption::FTUnit unit, AllMatches *arg)
    : FTDistanceMatches(info, from, unit, arg), distance2_(to) {}
  bool distanceMatches(unsigned int actual) const;

private:
  unsigned int distance2_;
};

#endif
