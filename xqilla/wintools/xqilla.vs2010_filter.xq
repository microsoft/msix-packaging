declare variable $projectFile external;
declare variable $outputPath external;
declare variable $msbversion := "4.0";
declare variable $sourcePath := doc($projectFile)/projects/variable[@name="sourcePath"];

declare function local:indent($n) 
{
  concat("&#xa;",string-join(for $i in (1 to $n) return " ", ""))
};

declare function local:windowsPath($path) as xs:string
{
  translate($path,"/","\\")
};

declare function local:getFilteredProjects()
{
  doc($projectFile)/projects/project[@name="xqilla"]
};

declare function local:getVsversion()
{
  (: Only be suitable when version >= 10.0 :)
  distinct-values(doc($projectFile)//visualstudioversion[fn:number(.) >= 10.0])
};

declare function local:getPlatforms($version) 
{
  if ($version eq "7.10") then ("Win32")
  else ("Win32", "x64")
};

declare function local:getOutputName($project, $vsversion)
{
  let $vsname := if($vsversion = "7.10") then "VC7.1" 
	         else if($vsversion = "8.00") then "VC8"
		 else if($vsversion = "10.0") then "VC10"
		 else "VC11"
  let $postfix := if($vsversion = "7.10") then ".vcproj.filters" 
      	       	  else if($vsversion = "8.00") then ".vcproj.filters"
		  else ".vcxproj.filters"
  return concat($outputPath, "/", $vsname, "/", $project/@name, $postfix)
};

declare function local:genFilters($project){
	for $filter in $project/files/filter
	let $name := $filter/@name
	let $guid := $filter/@guid
	return
		(local:indent(4),<Filter Include="{$name}" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
		{local:indent(6)}<UniqueIdentifier xmlns="http://schemas.microsoft.com/developer/msbuild/2003">{concat("{",$guid,"}")}</UniqueIdentifier>
		{local:indent(4)}</Filter>)
};

declare function local:getFilterName($file){
	data($file/../@name)
};

declare function local:generateIncFiles($project)
{
  for $file in $project/files//file return
    if (ends-with($file/@name,".h") or ends-with($file/@name,".hpp")) then 
      (local:indent(4),<ClInclude xmlns="http://schemas.microsoft.com/developer/msbuild/2003" Include="{local:windowsPath(concat($sourcePath,$file/@name))}">
	 {local:indent(6)}<Filter xmlns="http://schemas.microsoft.com/developer/msbuild/2003">{local:getFilterName($file)}</Filter>
       {local:indent(4)}</ClInclude>)
    else ()
};

declare function local:getConfiguration($project)
{
  for $compType in ("Debug", "Release") return
    for $config in $project/configuration return
      concat($config, $compType)    
};

declare function local:generateSrcFiles($project)
{ 
  for $file in $project/files//file return
      if (ends-with($file/@name,".rc") or ends-with($file/@name,".def") or ends-with($file/@name,".h") or ends-with($file/@name,".hpp")) then ()
      else 
        (local:indent(4),<ClCompile xmlns="http://schemas.microsoft.com/developer/msbuild/2003" Include="{local:windowsPath(concat($sourcePath,$file/@name))}">
	 {local:indent(6)}<Filter xmlns="http://schemas.microsoft.com/developer/msbuild/2003">{local:getFilterName($file)}</Filter>
	 {local:indent(4)}</ClCompile> )
};

declare function local:generateRCFiles($project)
{
  for $file in $project/files//file
  let $vsversion := local:getVsversion()
      return
       if (ends-with($file/@name,".rc")) then
	    (local:indent(2),<ItemGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	       {local:indent(4)}<ResourceCompile Include="{local:windowsPath(concat($sourcePath, $file/@name))}">
	         {local:indent(6)}<Filter xmlns="http://schemas.microsoft.com/developer/msbuild/2003">{local:getFilterName($file)}</Filter>
	       {for $platform in local:getPlatforms($vsversion) return
	          for $config in local:getConfiguration($project) return(
		    local:indent(6), <PreprocessorDefinitions Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}">{concat(if ($config="Debug") then "_DEBUG" else "NDEBUG",";","$(NoInherit))",";%(PreprocessorDefinitions)")}</PreprocessorDefinitions>)
	       }
	       {local:indent(4)}</ResourceCompile> 
	    {local:indent(2)}</ItemGroup>	    
	    )
	else ()
	
 
};

declare function local:genFiles($project){
    	 local:indent(2),<ItemGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	 {local:generateIncFiles($project)}
    	 {local:indent(2)}</ItemGroup>,
    	 local:indent(2),<ItemGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	 {local:generateSrcFiles($project)}
    	 {local:indent(2)}</ItemGroup>,
	 local:generateRCFiles($project) 
};

for $vsversion in local:getVsversion()
for $project in local:getFilteredProjects() 
return 
put(
<Project ToolsVersion="{$msbversion}" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	{local:indent(2)}<ItemGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">		
		{local:genFilters($project)}
	{local:indent(2)}</ItemGroup>
	{local:genFiles($project)}
{local:indent(0)}</Project>, local:getOutputName($project,$vsversion)
)
