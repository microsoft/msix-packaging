(:
*  IMPORTANT: this file is shared (identical) between the BDB XML and XQilla
*  projects.  It should not be changed independently or in any way that makes
*  it project-dependent.
*
* TBD:
*  1. Could drive the remaining variables, including Visual Studio
*  variables, off of the xml file and provide the XML input 
*  file as an external variable to the query.  E.g. some projects
*  may want to parameterize the optimization and warnings levels
*  2. Generate AdditionalIncludeDirectories based on the <depends> elements for 
*  projects rather than just including "all" 
:)
declare variable $projectFile external;
declare variable $outputPath external;
declare variable $projectDoc := doc($projectFile);
declare variable $sourcePath := $projectDoc/projects/variable[@name="sourcePath"];

(: Visual Studio Variables --relatively static :)
declare variable $debugOptLevel := "0";  (: VS -- no optimization :)
declare variable $releaseOptLevel := "2"; (: VS -- level 2 opt :)
declare variable $warnLevel := "3"; (: VS warning level :)
declare variable $appType := "1"; (: VS type 1 is a program :)
declare variable $dllType := "2";  (: VS type 2 is a DLL :)
declare variable $staticType := "4"; (: VS type 4 is a static lib :)
declare variable $debugInfo := "3"; (: VS debug information format :)
declare variable $libExtension := ".lib";

(: formatting :)
declare function local:indent($n)
{
  concat("&#xa;",string-join(for $i in (1 to $n) return " ", ""))
};

(: is it a DLL, Application, or static library? :)
declare function local:configurationType($project, $config)
{
  if ($project/type eq "library") then if (contains($config,"Static")) then
     $staticType else $dllType
  else if ($project/type eq "app") then $appType
  else error(QName("", "xqilla"),"configurationType: unknown project type")
};

(: "normalize" Windows file paths :)
declare function local:windowsPath($path) as xs:string
{
        translate($path,"/","\\")
};

(: debug vs release :)
declare function local:isDebug($config) as xs:boolean
{
  contains($config,"Debug")
};

declare function local:isRelease($config) as xs:boolean
{
  contains($config,"Release")
};

(: machine-target-dependent link options :)
declare function local:addLinkOptions($project, $platform, $config)
{
  let $machine := if (contains($platform,"Win32")) then "/machine:x86"
      	             else concat("/machine:",$platform)
  let $opt := string-join(($machine,$project/options/link[contains(@config,$config)])," ")
  return if (not($opt eq "")) then attribute{"AdditionalOptions"}{$opt}
         else ()
};

declare function local:addDebugInformation($config)
{
  attribute{"DebugInformationFormat"}{$debugInfo},	
   if (local:isDebug($config)) then attribute{"BasicRuntimeChecks"}{"3"} else ()
};

(: return "dll", "app", "static_lib", or "static_app" :)
declare function local:getProjectType($project, $config)
{
	if ($project/type eq "library") then if (contains($config,"Static")) then "static_lib"
	   else "dll"
	else if ($project/type eq "app") then if (contains($config,"Static")) then "static_app"
	   else "app"
	else error(QName("", "dbxml"),"getProjectType: unknown project type")	
};

declare function local:generateCompilerPreprocessorDefs($project, $config)
{
  let $ptype := local:getProjectType($project,$config)
  let $generic := $projectDoc/projects/preprocessor[@config="all" or contains($config,@config)]
  let $genericType := $projectDoc/projects/preprocessor[@config=$ptype]
  let $proj := $project/preprocessor[@config="all" or contains(@config,$config)]
  let $projType := $project/preprocessor[@config=$ptype]

  return string-join(($generic,$genericType,$proj,$projType),";")
};

(:
declare function local:generateResourcePreprocessorDefs($config)
{
  let $dbg := if (local:isDebug($config)) then "_DEBUG" else "NDEBUG"
  return concat($dbg,";_CRT_SECURE_NO_DEPRECATE=1")
};
:)

(: Optimization level :)
declare function local:optLevel($config)
{
  if (local:isDebug($config)) then $debugOptLevel
  else $releaseOptLevel
};

(: MDd is 3, MD is 2, MTd is 1, MT is 0 :)
declare function local:runtimeLibrary($config,$static as xs:boolean)
{
  if (local:isDebug($config)) then 
    if ($static) then "1" else"3" 
  else 
    if ($static) then "0" else"2" 
};

declare function local:getLibName($name, $config)
{
  $projectDoc/projects/library[@name=$name]/libname[@config=$config]
};

declare function local:makeStaticOutputFile($project, $config)
{
  attribute{"OutputFile"}
  {concat("$(OutDir)/", local:getLibName($project/@name, $config),".lib")
  }
};

declare function local:makeImportLibrary($project, $config)
{
  if ($project/type eq "library" and not(contains($config,"Static"))) then
    attribute{"ImportLibrary"}{concat("$(OutDir)/", local:getLibName($project/@name, $config),".lib")}
  else ()
};

declare function local:makeModuleDefinition($project, $config)
{
  if (not(empty($project/moddef))) then
    attribute{"ModuleDefinitionFile"}{local:windowsPath(concat($sourcePath,$project/moddef/@file))}
  else ()
};

declare function local:makeOutputPDBFile($project, $config)
{
  attribute{"ProgramDatabaseFile"}
  {
    if ($project/type eq "library") then
       concat("$(OutDir)/",  local:getLibName($project/@name, $config),".pdb")
    else
      concat("$(OutDir)/",if (not(empty($project/@outputpdb))) then $project/@outputpdb else $project/@name,".pdb")							    
  }
};

declare function local:makeOutputFile($project, $config)
{
  attribute{"OutputFile"}
  {
    if ($project/type eq "library") then
       concat("$(OutDir)/", local:getLibName($project/@name, $config), ".dll")
    else							    
      concat("$(OutDir)/",if (not(empty($project/@output))) then $project/@output else $project/@name,".exe")
  }
};

(: The simple thing is to add all libraries for all projects :)
declare function local:addLibraryDependencies($project,$config)
{
  attribute{"AdditionalDependencies"}{string-join(for $dep in $project/depends
   return concat(local:getLibName($dep, $config),".lib")," ")}
};

declare function local:makeLibraryDirectory($lib,$platform,$config,$vsversion)
{
   for $dir in $lib/platform[contains(@name,$platform) and contains(@vsver,$vsversion)]/config[$config=./@type]/libdir
   return 
      if (not(empty($lib/libbase[@vsver=$vsversion]))) then
   	concat($lib/libbase[@vsver=$vsversion],"/", $dir)
      else $dir
};

(: The simple thing is to add all libraries for all projects :)
declare function local:addLibraryDirectories($project,$platform,$config,$vsversion)
{
  attribute{"AdditionalLibraryDirectories"}{string-join(for $dep in $project/depends
   return local:makeLibraryDirectory($projectDoc/projects/library[@name=$dep],$platform,$config,$vsversion),";")}
};

(: The simple thing is to add all libraries for all projects :)
declare function local:addIncludeDirectories($project,$config)
{
  let $incref := for $inc in $project/include[@type="ref"] return $projectDoc/projects/include[@name=$inc]
  let $increl := $project/include[@type="rel"]
  return
   attribute{"AdditionalIncludeDirectories"}{string-join(($incref,$increl),",")}
};

(: look for project-specific tool, then a type-specific event :)
declare function local:generatePostBuildEvent($project, $config)
{
local:indent(6),<Tool>
    {attribute{"Name"}{"VCPostBuildEventTool"}}
    {if (not(empty($project/event[@name="postbuild"]))) then
           (attribute{"CommandLine"}{$project/event[@name="postbuild"]/command[@config=$config]},
           attribute{"Description"}{$project/event[@name="postbuild"]/description})
     else let $ev := $projectDoc/projects/event[@name="postbuild" and @type=$project/type]
           return if (not(empty($ev))) then 
	              (attribute{"CommandLine"}{replace($ev/command[@config=$config],"@pname@",if (not(empty($project/@output))) then $project/@output else $project/@name)},
           	      attribute{"Description"}{$ev/description})
           else ()
    }
</Tool>
};

declare function local:generateCustomBuildTool($project, $config)
{
local:indent(6),<Tool>
    {attribute{"Name"}{"VCCustomBuildTool"}}
    {if (not(empty($project/event[@name="custom"]))) then
        (attribute{"CommandLine"}{$project/event[@name="custom"]/command[contains(@config,$config)]},
    	attribute{"Outputs"}{$project/event[@name="custom"]/output})
	else ()}
</Tool>
};

(: 
 static build of libraries; at this time it does *not* add additional
 dependent libraries.  This means that applications need to include them
 for the time being.
:)
declare function local:generateConfigLibrarian($project, $platform, $config)
{
local:indent(6),<Tool>
  {attribute{"Name"}{"VCLibrarianTool"}}
  {local:addLinkOptions($project,$platform,$config)}
  {local:makeStaticOutputFile($project,$config)}
</Tool>
};

declare function local:generateConfigLinkerAndMidl($project, $platform, $config, $vsversion)
{
local:indent(6),<Tool>
  {attribute{"Name"}{"VCLinkerTool"}}
  {local:addLinkOptions($project, $platform, $config)}
  {local:addLibraryDependencies($project,$config)}
  {local:addLibraryDirectories($project,$platform,$config,$vsversion)}
  {local:makeOutputFile($project, $config)}
  {local:makeOutputPDBFile($project, $config)}
  {if (local:isDebug($config)) then attribute{"LinkIncremental"}{"2"} else attribute{"LinkIncremental"}{"1"}}
  {attribute{"GenerateDebugInformation"}{"TRUE"}}
  {attribute{"SuppressStartupBanner"}{"TRUE"}}
  {if (local:isDebug($config)) then attribute{"OptimizeReferences"}{"1"} else attribute{"OptimizeReferences"}{"2"}}
  {local:makeImportLibrary($project,$config)}
  {local:makeModuleDefinition($project,$config)}
  {attribute{"TargetMachine"}{"0"}}
</Tool>
};

declare function local:generateConfigCompiler($project, $platform, $config, $static as xs:boolean, $vsversion)
{
local:indent(6),<Tool>
  {attribute{"Name"}{"VCCLCompilerTool"}}
  {attribute{"Optimization"}{local:optLevel($config)}}
  {attribute{"MinimalRebuild"}{"TRUE"}}
  {if (local:isRelease($config)) then attribute{"InlineFunctionExpansion"}{"1"} else ()}
  {local:addIncludeDirectories($project,$config)}
  {attribute{"PreprocessorDefinitions"}{local:generateCompilerPreprocessorDefs($project, $config)}}
  {attribute{"StringPooling"}{"TRUE"}}
  {if (not(empty($project/options/rtti))) then attribute{"RuntimeTypeInfo"}{"TRUE"} else ()}
  {attribute{"RuntimeLibrary"}{local:runtimeLibrary($config,$static)}}
  {if (local:isRelease($config)) then attribute{"EnableFunctionLevelLinking"}{"TRUE"} else ()}
  {attribute{"UsePrecompiledHeader"}{"0"}}
  {if ($project/@name eq "dbxml") then attribute{"PrecompiledHeaderThrough"}{"DbXmlInternal.hpp"} else ()}
  {attribute{"PrecompiledHeaderFile"}{concat("./$(IntDir)/",$project/@name,".pch")}}
  {attribute{"AssemblerListingLocation"}{"./$(IntDir)/"}}
  {attribute{"ObjectFile"}{"./$(IntDir)/"}}
  {attribute{"WarningLevel"}{$warnLevel}}
  {attribute{"SuppressStartupBanner"}{"TRUE"}}
  {local:addDebugInformation($config)}
  {attribute{"CompileAs"}{"0"}}
</Tool>
};

declare function local:generateConfigBoilerplate($config)
{
local:indent(6),<Tool Name="VCPreBuildEventTool"/>,
local:indent(6),<Tool Name="VCPreLinkEventTool"/>,
local:indent(6),<Tool Name="VCResourceCompilerTool"/>,
local:indent(6),<Tool Name="VCXMLDataGeneratorTool"/>,
local:indent(6),<Tool Name="VCManagedWrapperGeneratorTool"/>,
local:indent(6),<Tool Name="VCAuxiliaryManagedWrapperGeneratorTool"/>
};

(: use "platform/configuration" :)
declare function local:generateOutputDirectory($platform,$config,$vsversion)
{
  let $outputBase := $projectDoc/projects/variable[@name=concat("outputBase.", $vsversion)]
  return attribute{"OutputDirectory"}{local:windowsPath(concat($outputBase,"$(PlatformName)","/",$config))}
};

declare function local:generateConfig($project, $platform, $config, $vsversion)
{
local:indent(4),<Configuration>
    {attribute{"Name"}{concat($config,"|",$platform)}}
    {local:generateOutputDirectory($platform,$config,$vsversion)}
    {attribute{"IntermediateDirectory"}{concat("./$(OutDir)/",$project/@name)}}
    {attribute{"ConfigurationType"}{local:configurationType($project,$config)}}
    {attribute{"UseOfMFC"}{"0"}}
    {attribute{"ATLMinimizesCRunTimeLibraryUsage"}{"FALSE"}}
    {attribute{"CharacterSet"}{"2"}}
    {local:generateConfigBoilerplate($config)}
    {local:generateConfigCompiler($project, $platform, $config,false(),$vsversion)}
    {local:generateConfigLinkerAndMidl($project, $platform, $config, $vsversion)}
    {local:generatePostBuildEvent($project,if (local:isDebug($config)) then "Debug" else "Release")}
    {local:generateCustomBuildTool($project,if (local:isDebug($config)) then "Debug" else "Release")}
  </Configuration>
};

declare function local:generateStaticConfig($project, $platform, $config, $vsversion)
{
  local:indent(4),<Configuration>
    {attribute{"Name"}{concat($config,"|",$platform)}}
    {local:generateOutputDirectory($platform,$config,$vsversion)}
    {attribute{"IntermediateDirectory"}{concat("./$(OutDir)/",$project/@name)}}
    {attribute{"ConfigurationType"}{local:configurationType($project,$config)}}
    {attribute{"UseOfMFC"}{"0"}}
    {attribute{"ATLMinimizesCRunTimeLibraryUsage"}{"FALSE"}}
    {attribute{"CharacterSet"}{"2"}}
    {local:generateConfigBoilerplate($config)}
    {local:generateConfigCompiler($project, $platform, $config,true(),$vsversion)}
    {if (contains($project/type,"lib")) then 
         local:generateConfigLibrarian($project, $platform, $config)
     else
         local:generateConfigLinkerAndMidl($project, $platform, $config, $vsversion)
    }
  </Configuration>
};

declare function local:generateRcFile($file, $vsversion)
{
	local:indent(4),<File RelativePath="{local:windowsPath(concat($sourcePath,$file/@name))}">
	{ for $platform in local:getPlatforms($vsversion) return 
            for $config in ("Debug","Release") return
              (local:indent(6),<FileConfiguration Name="{concat($config,"|",$platform)}">
                  <Tool Name="VCResourceCompilerTool" PreprocessorDefinitions="{concat(if ($config="Debug") then "_DEBUG" else "NDEBUG",";","$(NoInherit))")}"/>
	      {local:indent(6)}</FileConfiguration>)
        }
	{local:indent(4)}</File>
};

declare function local:generateFilesNoFilter($project, $vsversion)
{
    for $file in $project/files/file
        return  if (ends-with($file/@name,".rc")) then local:generateRcFile($file, $vsversion)
	  else  (local:indent(4),<File RelativePath="{local:windowsPath(concat($sourcePath,$file/@name))}"/>)
};

declare function local:generateFilesWithFilter($project,$filter,$vsversion)
{
    for $file in $project/files/filter[@name=$filter]/file
        return if (ends-with($file/@name,".rc")) then local:generateRcFile($file, $vsversion)
	  else (local:indent(6),<File RelativePath="{local:windowsPath(concat($sourcePath,$file/@name))}"/>)
};

declare function local:generateFiles($project, $vsversion)
{
  let $filters := $project/files/filter/@name
  return if (empty($filters)) then
          local:generateFilesNoFilter($project, $vsversion)
     else
         for $filter in $filters
         return (local:indent(4),<Filter Name="{$filter}" Filter="">
               {local:generateFilesWithFilter($project,$filter,$vsversion)}
         {local:indent(4)}</Filter>)
};

declare function local:getGuid($project)
{
	concat("{",$project/@guid,"}")	
};

declare function local:getPlatforms($version)
{
	if ($version eq "7.10") then ("Win32")
	else ("Win32", "x64")
};

declare function local:getOutputName($project, $vsversion)
{
  let $vsname := if($vsversion = "7.10") then "VC7.1"
      else if($vsversion = "8.00") then  "VC8"
      else "VC9"
  return
    concat($outputPath, "/", $vsname, "/", $project/@name, ".vcproj")
};

declare function local:getConfigurations($project)
{
    for $compType in ("Debug","Release") return	
        for $config in $project/configuration return concat($config,$compType)
};

for $vsversion in distinct-values($projectDoc//visualstudioversion)
for $project in $projectDoc/projects/project
let $static := contains($project/@name,"static")
let $proj := $project
return
if ($vsversion = "10.0") then ()
else if ($vsversion = "11.0") then ()
else (
put(<VisualStudioProject
   ProjectType="Visual C++"
   Version="{$vsversion}"
   Name="{string($project/@name)}"
   ProjectGUID="{local:getGuid($project)}">
   {local:indent(2)}<Platforms>
   {local:indent(4)}<Platform Name="Win32"/>
   {local:indent(4)}<Platform Name="x64"/>
   {local:indent(2)}</Platforms>
   {local:indent(2)}<Configurations>
    {
    for $platform in local:getPlatforms($vsversion) return 
    for $config in local:getConfigurations($proj) return if (contains($config, "Static")) then local:generateStaticConfig($project, $platform, $config, $vsversion) else local:generateConfig($project, $platform, $config, $vsversion)
    }
  {local:indent(2)}</Configurations>
  {local:indent(2)}<References/>
  {local:indent(2)}<Files>
    {local:generateFiles($proj, $vsversion)}
  {local:indent(2)}</Files>
  {local:indent(2)}<Globals/>
{"&#xa;"}</VisualStudioProject>, local:getOutputName($project, $vsversion))
)
