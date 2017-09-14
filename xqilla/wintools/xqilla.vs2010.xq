(:declare default element namespace "http://schemas.microsoft.com/developer/msbuild/2003";:)
declare variable $projectFile external;
declare variable $outputPath external;
declare variable $xmlversion := "1.0";
declare variable $xmlencoding := "utf-8";
declare variable $msbversion := "4.0";
declare variable $msbnamespace := "http://schemas.microsoft.com/developer/msbuild/2003";
declare variable $msbdefprops := '$(VCTargetsPath)\Microsoft.Cpp.Default.props';
declare variable $msbprops := '$(VCTargetsPath)\Microsoft.Cpp.props';
declare variable $msbtargets := "$(VCTargetsPath)\Microsoft.Cpp.targets";
declare variable $mobile := xs:boolean(doc($projectFile)/projects/variable[@name="mobile"]);
declare variable $debugOptLevel := "Disabled";
declare variable $releaseOptLevel := "MaxSpeed";
declare variable $warnLevel := "Level3";
declare variable $debugInfo := "ProgramDatabase";
declare variable $sourcePath := doc($projectFile)/projects/variable[@name="sourcePath"];
declare variable $dllType := "dll";
declare variable $appType := "Application";
declare variable $libprop := "library.props";

declare function local:genImportProperty($libprop)
{
  local:indent(2),<ImportGroup>
    {local:indent(4)}<Import Project="{$libprop}"/>
  local:indent(2)</ImportGroup>
};

declare function local:indent($n) 
{
  concat("&#xa;",string-join(for $i in (1 to $n) return " ", ""))
};

declare function local:getPlatforms($version) 
{
  if ($version eq "7.10") then ("Win32")
  else ("Win32", "x64")
};

declare function local:getGuid($project)
{
  concat("{",$project/@guid,"}")
};

declare function local:generateProjectConfigurations($project, $vsversion)
{
  local:indent(2),<ItemGroup Label="ProjectConfigurations" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  {for $config in local:getConfiguration($project) return 
    for $platform in local:getPlatforms($vsversion) return 
       (local:indent(4),<ProjectConfiguration Include="{concat($config, "|",$platform)}">
       {local:indent(6)}<Configuration>{$config}</Configuration>
       {local:indent(6)}<Platform>{$platform}</Platform>
       {local:indent(4)}</ProjectConfiguration>)
  }
  {local:indent(2)}</ItemGroup>
};

declare function local:generateGlobalProperty($project)
{
  local:indent(2),<PropertyGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003" Label="Globals">
    {local:indent(2)}<ProjectGuid>{local:getGuid($project)}</ProjectGuid> 	
  {local:indent(2)}</PropertyGroup>
};


declare function local:configurationType($project, $config)
{
  if ($project/type eq "library") then if (contains($config,"Static")) then
    "StaticLibrary" else "DynamicLibrary"
  else if ($project/type eq "app") then "Application"
  else error(QName("","xqilla"),"configurationType:unkown project type")
};

declare function local:getPlatformToolset($vsversion)
{
  if ($vsversion = "10.0") then "v100"
  else "v110"
};

declare function local:generateConfigurations($project, $vsversion)
{
  for $platform in local:getPlatforms($vsversion) return
    for $config in local:getConfiguration($project) 
      let $configType := local:configurationType($project,$config)
      let $platformToolset := local:getPlatformToolset($vsversion)	    
      return 
      (local:indent(2),<PropertyGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003" 
                        Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}"
			Label="Configuration">
        {local:indent(4)}<ConfigurationType>{$configType}</ConfigurationType>
	{local:indent(4)}<UseOfMfc>false</UseOfMfc>
	{local:indent(4)}<CharacterSet>MultiByte</CharacterSet>
	{local:indent(4)}<PlatformToolset>{$platformToolset}</PlatformToolset>
      {local:indent(2)}</PropertyGroup>
      )
};

declare function local:genExtensionSettings()
{
  local:indent(2),<ImportGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003" Label="ExtensionSettings">
  {local:indent(2)}</ImportGroup>
};

declare function local:genExtensionTargets()
{
  local:indent(2),<ImportGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003" Label="ExtensionTargets">
  {local:indent(2)}</ImportGroup>
};

declare variable $msbuserprops := "$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props";
declare variable $msbuserpropsexit := "exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')";

declare function local:genPropertySheets($project, $vsversion) 
{
  for $platform in local:getPlatforms($vsversion) return 
    for $config in local:getConfiguration($project) return
      (local:indent(2),<ImportGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003"
                         Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}"
                         Label="PropertySheets">
	{local:indent(4)}<Import xmlns="http://schemas.microsoft.com/developer/msbuild/2003"
                           Project="{$msbuserprops}" Condition="{$msbuserpropsexit}" Label="LocalAppDataPlatform" />
      {local:indent(4)}</ImportGroup>	
      )
};


declare function local:windowsPath($path) as xs:string
{
  translate($path,"/","\\")
};

declare function local:generateOutDir($config, $vsversion)
{
  let $outputBase := doc($projectFile)/projects/variable[@name=concat("outputBase.", $vsversion)]
  return local:windowsPath(concat($outputBase, "$(Platform)", "/", $config, "/"))
};

(:becaues zlib has different 32 and 64 bits platform libraries, so use another getLibName:)
declare function local:getLibName($name, $config)
{
  doc($projectFile)/projects/library[@name=$name]/libname[@config=$config and not(@platform eq "x64")]/text()
};

declare function local:getLibNameWithPlatform($name, $config, $platform)
{
  let $libnamePlatform := doc($projectFile)/projects/library[@name=$name]/libname[@config=$config and @platform=$platform]/text()
  let $libname := doc($projectFile)/projects/library[@name=$name]/libname[@config=$config and not(@platform)]/text()
  return if ($libnamePlatform) then ($libnamePlatform)
  else ($libname)
};

declare function local:genMacros($project, $vsversion)
{ 
    local:genDynamicMacros($project, $vsversion)
};  

declare function local:genTargetName($project,$config)
{
  if ($project/type eq "library") then 
     local:getLibName($project/@name, $config)
  else
     if (not(empty($project/@output))) then data($project/@output) else data($project/@name)
};

declare function local:genTargetExt($project, $config)
{
  if ($project/type eq "library") then 
    if (contains($config, "Static")) then ".lib"
    else ".dll"
  else ".exe"
};

(: no static variable usage now :)
declare function local:genDynamicMacros($project, $vsversion)
{
  local:indent(2),<PropertyGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
    {local:indent(4)}<_ProjectFileVersion>10.0.30319.1</_ProjectFileVersion>
    {for $platform in local:getPlatforms($vsversion) return
      for $config  in local:getConfiguration($project) return
        (local:indent(4),<OutDir xmlns="http://schemas.microsoft.com/developer/msbuild/2003"
                           Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}">
          {local:generateOutDir($config, $vsversion)}</OutDir>,
	 local:indent(4),<IntDir xmlns="http://schemas.microsoft.com/developer/msbuild/2003"
                           Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}"> 
	  {concat("./$(OutDir)",$project/@name,"\")}</IntDir>,
         local:indent(4),<LinkIncremental xmlns="http://schemas.microsoft.com/developer/msbuild/2003"
                           Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}">false</LinkIncremental>,
	 local:indent(4),<TargetName xmlns="http://schemas.microsoft.com/developer/msbuild/2003"
                           Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}">{local:genTargetName($project,$config)}</TargetName>,
	 local:indent(4),<TargetExt xmlns="http://schemas.microsoft.com/developer/msbuild/2003"
                           Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}">{local:genTargetExt($project, $config)}</TargetExt>
	)
    }
  {local:indent(2)}</PropertyGroup>
};

declare function local:isDebug($config) as xs:boolean
{
  contains($config,"Debug")
};

declare function local:isRelease($config) as xs:boolean
{
  contains($config,"Release")
};

declare function local:optLevel($config)
{
  if (local:isDebug($config)) then $debugOptLevel
  else $releaseOptLevel
};

declare function local:addIncludeDirectories($project,$config)
{
  let $incref := for $inc in $project/include[@type="ref"] return doc($projectFile)/projects/include[@name=$inc]
  let $increl := $project/include[@type="rel"]
  return
    translate(concat(string-join($incref,";"),
		     if (not(empty($increl)) and not(empty($incref))) then ";" else "",
			    string-join($increl,";"),";","%(AdditionalIncludeDirectories)"),",",";")
};

declare function local:getProjectType($project, $config)
{ 
  if ($project/type eq "library") then if (contains($config,"Static")) then "static_lib"
    else "dll"
  else if ($project/type eq "app") then if (contains($config,"Static")) then "static_app"
  else "app"
  else error(QName("","dbxml"),"getProjectType:unkown project type")
};

declare function local:generateCompilerPreprocessorDefs($project,$config) 
{ 
  let $ptype := local:getProjectType($project, $config)
  let $generic := doc($projectFile)/projects/preprocessor[@config="all" or contains($config,@config)]
  let $type := doc($projectFile)/projects/preprocessor[@config=$ptype]
  let $proj := $project/preprocessor[@config="all" or @config=$config]
  let $projType := $project/preprocessor[@config=$ptype]
  return 
    concat(string-join(($generic,$type,$proj,$projType),";"),";","%(PreprocessorDefinitions)")
};

declare function local:runtimeLibrary($config)
{
  if (local:isDebug($config)) then
    if (contains($config,"Static")) then "MultiThreadedDebug" else "MultiThreadedDebugDLL"
  else 
    if (contains($config,"Static")) then "MultiThreaded" else "MultiThreadedDLL"
};

declare function local:makeObjectFile($project,$config)
{
  concat("$(OutDir)", if (not(empty($project/@output))) then $project/@output else $project/@name,"/")
};

declare function local:addDebugInformation($config)
{
   local:indent(6),<DebugInformationFormat xmlns="http://schemas.microsoft.com/developer/msbuild/2003">{$debugInfo}</DebugInformationFormat>,
   if (local:isDebug($config)) then (local:indent(6),<BasicRuntimeChecks xmlns="http://schemas.microsoft.com/developer/msbuild/2003">EnableFastChecks</BasicRuntimeChecks>) else ()
};

declare function local:addLinkOptions($project, $platform, $config)
{
  let $machine := 
    if (contains($platform,"Win32")) then "/machine:x86"
    else concat("/machine:",$platform)
  let $opt := string-join(($machine,$project/options/link[contains(@config,$config)])," ")
  return if (not($opt eq " ")) then (local:indent(6),<AdditionalOptions xmlns="http://schemas.microsoft.com/developer/msbuild/2003">{string-join(($opt,"%(AdditionalOptions)")," ")}</AdditionalOptions> )
         else ()
};

declare function local:addLibraryDependencies($project,$config,$platform)
{
    string-join(for $dep in $project/depends return concat(local:getLibNameWithPlatform($dep,$config,$platform),".lib"), ";")
};


declare function local:addProjectDependencies($project)
{	
   for $mydep in $project/references
	   let $mytemp := (  
		if ($mydep eq "XercesLib") then '{152CE948-F659-4206-A50A-1D2B9658EF96}'
		else if ($mydep eq "xqilla") then '{63E85107-41E0-4FC7-8083-40E85861B426}'
		else 
		lower-case(concat("{",doc($projectFile)/projects/project[@name=$mydep]/@guid,"}"))
	   )
	   return
	   (local:indent(4), <ProjectReference Condition="exists('{concat($mydep,".vcxproj")}')" Include="{concat($mydep,".vcxproj")}" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	    {local:indent(6)}<Project>{$mytemp}</Project>
	    {local:indent(6)}<ReferenceOutputAssembly>false</ReferenceOutputAssembly>
           {local:indent(4)}</ProjectReference>)
};

declare function local:makeLibraryDirectory($lib,$platform,$config, $vsversion)
{
  for $dir in $lib/platform[contains(@name,$platform)]/config[$config=./@type]/libdir
    return 
      if (not(empty($lib/libbase[@vsver=$vsversion]))) then 
        concat($lib/libbase[@vsver=$vsversion],"/",$dir)
      else $dir
};

declare function local:addLibraryDirectories($project, $platform, $config, $vsversion)
{
  string-join((for $dep in $project/depends return local:makeLibraryDirectory(doc($projectFile)/projects/library[@name=$dep],$platform,$config, $vsversion),"../lib"),";")
};

declare function local:makeOutputPDBFile($project,$config)
{
  if ($project/type eq "library") then
    concat("$(OutDir)",local:getLibName($project/@name,$config),".pdb")
  else
    concat("$(OutDir)",if (not(empty($project/@output))) then $project/@output else $project/@name,".pdb")
};

declare function local:makeImportLibrary($project,$config)
{
  if ($project/type eq "library" and not(contains($config,"Static"))) then
    (local:indent(6),<ImportLibrary xmlns="http://schemas.microsoft.com/developer/msbuild/2003">{concat("$(OutDir)",local:getLibName($project/@name,$config),".lib")}</ImportLibrary>)
  else ()
};

declare function local:makeModuleDefinition($project,$config)
{
  if (not(empty($project/moddef))) then
    let $temp := concat($sourcePath,$project/moddef/@file)
    return
    (local:indent(6),<ModuleDefinitionFile xmlns="http://schemas.microsoft.com/developer/msbuild/2003">{local:windowsPath($temp)}</ModuleDefinitionFile>)
  else ()
};

declare function local:generateCustomBuildTool($project,$config)
{ 
    if (not(empty($project/event[@name="custom"]))) then 
        let $commandtext := $project/event[@name="custom"]/command[contains(@config,$config)]/text()
        let $outputs := concat($project/event[@name="custom"]/output,";%(Outputs)")
        return (	  
            local:indent(4),<CustomBuildStep xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	    {local:indent(6)}<Command>{$commandtext}</Command>
	    {local:indent(6)}<Outputs>{$outputs}</Outputs>
	    {local:indent(4)}</CustomBuildStep>)
    else if (not(empty($project/event[@name="postbuild"]))) then 
	let $commandtext := $project/event[@name="postbuild"]/command[contains(@config,$config)]/text()
        let $message := $project/event[@name="postbuild"]/description
        return (
	    local:indent(4),<PostBuildEvent xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	    {local:indent(6)}<Command>{$commandtext}</Command>
	    {local:indent(6)}<Message>{$message}</Message>
	    {local:indent(4)}</PostBuildEvent>)
     else ()
};


declare function local:genDefinition($project,$vsversion)
{
    local:genDynamicDefinition($project,$vsversion)
};


declare function local:generateConfigCompiler($project,$platform,$config)
{
          local:indent(4),<ClCompile xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	  {local:indent(6)}<Optimization>{local:optLevel($config)}</Optimization>
	  {local:indent(6)}<MinimalRebuild>true</MinimalRebuild>
	  {if (local:isRelease($config)) then (local:indent(6),<InlineFunctionExpansion>OnlyExplicitInline</InlineFunctionExpansion>) else ()}
	  {local:indent(6)}<AdditionalIncludeDirectories>{local:addIncludeDirectories($project,$config)}</AdditionalIncludeDirectories>
	  {local:indent(6)}<PreprocessorDefinitions>{local:generateCompilerPreprocessorDefs($project,$config)}</PreprocessorDefinitions>
	  {local:indent(6)}<StringPooling>true</StringPooling>
          {local:indent(6)}<RuntimeLibrary>{local:runtimeLibrary($config)}</RuntimeLibrary>
	  {if (local:isRelease($config)) then (local:indent(6),<FunctionLevelLinking>true</FunctionLevelLinking>) else ()}
	  {local:indent(6)}<PrecompiledHeader>
	  {local:indent(6)}</PrecompiledHeader>
	  {local:indent(6)}<PrecompiledHeaderOutputFile>{concat("./$(OutDir)", $project/@name, ".pch")}</PrecompiledHeaderOutputFile>
	  {local:indent(6)}<AssemblerListingLocation>{local:makeObjectFile($project,$config)}</AssemblerListingLocation>
	  {local:indent(6)}<ObjectFileName>{local:makeObjectFile($project,$config)}</ObjectFileName>
          {local:indent(6)}<WarningLevel>{$warnLevel}</WarningLevel>
          {local:indent(6)}<SuppressStartupBanner>true</SuppressStartupBanner>
          {local:addDebugInformation($config)}
	  {local:indent(6)}<CompileAs>Default</CompileAs>
	{local:indent(4)}</ClCompile>

};
declare function local:generateDynamicLink($project, $platform, $config, $vsversion)
{
	local:indent(4),<Link xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	{local:addLinkOptions($project,$platform,$config)}
	{local:indent(6)}<AdditionalDependencies>{local:addLibraryDependencies($project,$config,$platform)};%(AdditionalDependencies)</AdditionalDependencies>
        {local:indent(6)}<AdditionalLibraryDirectories>{local:addLibraryDirectories($project,$platform,$config,$vsversion)};%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
	{local:indent(6)}<OutputFile>$(OutDir)$(TargetName)$(TargetExt)</OutputFile>
	{local:indent(6)}<ProgramDatabaseFile>{local:makeOutputPDBFile($project,$config)}</ProgramDatabaseFile>
	{local:indent(6)}<GenerateDebugInformation>true</GenerateDebugInformation>
	{local:indent(6)}<SuppressStartupBanner>true</SuppressStartupBanner>
        {local:makeImportLibrary($project,$config)}
	{local:indent(6)}<OptimizeReferences>true</OptimizeReferences>
	{local:makeModuleDefinition($project,$config)}
	{local:indent(6)}<TargetMachine>{if (contains($platform,"(ARMV4)")) then "MachineARM" else "NotSet"}</TargetMachine>
	{local:indent(4)}</Link>
};

declare function local:generateConfigLibrarian($project,$platform,$config)
{
  local:indent(4),<Lib xmlns="http://schemas.microsoft.com/developer/msbuild/2003" >
    {local:indent(6)}<OutputFile>$(OutDir)$(TargetName)$(TargetExt)</OutputFile>
    {local:addLinkOptions($project,$platform,$config)}
  {local:indent(4)}</Lib>
};


declare function local:generateStaticLink($project,$platform,$config,$vsversion)
{ 
  if (contains($project/type,"lib")) then
    local:generateConfigLibrarian($project,$platform,$config)
  else 
    local:generateDynamicLink($project,$platform,$config, $vsversion)
};

declare function local:genDynamicDefinition($project,$vsversion)
{
  for $platform in local:getPlatforms($vsversion) return
    for $config in local:getConfiguration($project) 
    let $static := contains($config, "Static")
    return
    (
      local:indent(2),<ItemDefinitionGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003"
                           Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}"> 
	{local:generateConfigCompiler($project,$platform,$config)}
        {if ($static) then local:generateStaticLink($project,$platform,$config, $vsversion)
        else local:generateDynamicLink($project,$platform,$config,$vsversion)}
	{local:generateCustomBuildTool($project,$config)}
      {local:indent(2)}</ItemDefinitionGroup>
    )
};

declare function local:genStaticDefinition($project, $vsversion)
{
  for $platform in local:getPlatforms($vsversion) return
    for $config in local:getConfiguration($project) return
    (
      local:indent(2),<ItemDefinitionGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003"
                           Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}"> 
	{local:generateConfigCompiler($project,$platform,$config)}
	{local:generateStaticLink($project,$platform,$config,$vsversion)}
	{local:generateCustomBuildTool($project,$config)}
      {local:indent(2)}</ItemDefinitionGroup>
    )
};

declare function local:generateIncFilesNoFilter($project)
{
  for $file in $project/files//file return
    if (ends-with($file/@name,".h") or ends-with($file/@name,".hpp")) then 
      (local:indent(4),<ClInclude xmlns="http://schemas.microsoft.com/developer/msbuild/2003" Include="{local:windowsPath(concat($sourcePath,$file/@name))}" />)
    else ()
};

declare function local:generateSrcFilesNoFilter($project)
{ 
  for $file in $project/files//file return
      if (ends-with($file/@name,".rc") or ends-with($file/@name,".def") or ends-with($file/@name,".h") or ends-with($file/@name,".hpp")) then ()
      else (local:indent(4),<ClCompile xmlns="http://schemas.microsoft.com/developer/msbuild/2003" Include="{local:windowsPath(concat($sourcePath,$file/@name))}"/>)		  
};


declare function local:generateRcFilesNoFilter($project, $vsversion)
{
  for $file in $project/files//file return
       if (ends-with($file/@name,".rc")) then
	    (local:indent(2),<ItemGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	       {local:indent(4)}<ResourceCompile Include="{substring-after($file/@name,"/")}">
	       {for $platform in local:getPlatforms($vsversion) return
	          for $config in local:getConfiguration($project) return(
		    local:indent(6), <PreprocessorDefinitions Condition="'$(Configuration)|$(Platform)'=={concat("'",$config,"|",$platform,"'")}">{concat(if ($config="Debug") then "_DEBUG" else "NDEBUG",";","$(NoInherit))",";%(PreprocessorDefinitions)")}</PreprocessorDefinitions>)
	       }
	       {local:indent(4)}</ResourceCompile> 
	    {local:indent(2)}</ItemGroup>	    
	    )
	else ()
	
 
};
	

declare function local:generateDefFilesNoFilter($project)
{
  for $file in $project/files//file return
    if (ends-with($file/@name,".def")) then 
      (
         local:indent(2),<ItemGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
	   {local:indent(4)}<None Include="{substring-after($file/@name,"/")}" />
	 {local:indent(2)}</ItemGroup>
      )
    else ()
};

declare function local:generateFilesWithFilter($project,$filter)
{
<a/>
};

declare function local:genFiles($project,$vsversion)
{
    local:indent(2),<ItemGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
    {local:generateIncFilesNoFilter($project)}
    {local:indent(2)}</ItemGroup>,
    local:indent(2),<ItemGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
    {local:generateSrcFilesNoFilter($project)}
    {local:indent(2)}</ItemGroup>,
    local:generateDefFilesNoFilter($project),
    local:generateRcFilesNoFilter($project,$vsversion)
};


declare function local:getConfiguration($project)
{
  for $compType in ("Debug", "Release") return
    for $config in $project/configuration return
      concat($config, $compType)    
};

declare function local:getOutputName($project, $vsversion)
{
  let $vsname := if($vsversion = "7.10") then "VC7.1" 
	         else if($vsversion = "8.00") then "VC8"
		 else if($vsversion = "9.00") then "VC9"
		 else if($vsversion = "10.0") then "VC10"
		 else "VC11"
  let $postfix := if($vsversion = "7.10") then ".vcproj" 
      	       	  else if($vsversion = "8.00") then ".vcproj"
		  else if($vsversion = "9.00") then ".vcproj"
		  else ".vcxproj"
  return concat($outputPath, "/", $vsname, "/", $project/@name, $postfix)
};

declare function local:getVsversion()
{
  (: Only be suitable when version >= 10.0 :)
  distinct-values(doc($projectFile)//visualstudioversion[fn:number(.) >= 10.0])
};

declare function local:getProjects()
{
  doc($projectFile)/projects/project
};

for $vsversion in local:getVsversion()
for $project in  local:getProjects()
return
put(
<Project DefaultTargets="Build" ToolsVersion="{$msbversion}" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  {local:generateProjectConfigurations($project, $vsversion)}
  {local:generateGlobalProperty($project)}
  {local:indent(2)}<Import Project="{$msbdefprops}" />
  {local:generateConfigurations($project, $vsversion)}
  {local:indent(2)}<Import Project="{$msbprops}" />
  {local:genExtensionSettings()}
  {local:genPropertySheets($project, $vsversion)}
  {local:indent(2)}<PropertyGroup Label="UserMacros" />
  {local:genMacros($project, $vsversion)}
  {local:genDefinition($project,$vsversion)}
  {local:genFiles($project,$vsversion)}
  {local:indent(2)}<Import Project="{$msbtargets}" />
  {local:genExtensionTargets()}
  {local:indent(2)}<ItemGroup xmlns="http://schemas.microsoft.com/developer/msbuild/2003">	
  {local:addProjectDependencies($project)}
  {local:indent(2)}</ItemGroup>
{local:indent(0)}</Project>, local:getOutputName($project, $vsversion))
