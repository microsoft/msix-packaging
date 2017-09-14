(:
 : Copyright (c) 2004-2009
 :     Oracle. All rights reserved.
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 :     http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
 :)

module namespace core = "http://xqilla.sourceforge.net/core";

declare function core:to($a as xs:integer?, $b as xs:integer?) as xs:decimal*
{
  if(empty($a) or empty($b)) then ()
  else core:to_helper($a treat as xs:decimal, $b treat as xs:decimal)
};

declare function core:to_helper($a as xs:decimal, $b as xs:decimal) as xs:decimal*
{
  if($a gt $b) then () else ($a, core:to_helper($a + 1, $b))
};

