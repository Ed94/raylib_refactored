$path_root     = git rev-parse --show-toplevel
$path_scripts  = $PSScriptRoot

$target_arch        = Join-Path $path_scripts 'helpers/target_arch.psm1'
$devshell           = Join-Path $path_scripts 'helpers/devshell.ps1'
$incremental_checks = Join-Path $path_scripts 'helpers/incremental_checks.ps1'
$vendor_toolchain   = Join-Path $path_scripts 'helpers/vendor_toolchain.ps1'

Import-Module $target_arch

#region Arguments
$vendor           = $null
$optimize         = $null
$debug 	          = $null
$analysis	      = $false
$dev              = $false
$verbose          = $null
$force_cpp        = $false

[array] $vendors = @( "clang", "msvc" )

# This is a really lazy way of parsing the args, could use actual params down the line...

if ( $args ) { $args | ForEach-Object {
switch ($_){
 { $_ -in $vendors }   { $vendor    = $_; break }
 "optimize"            { $optimize  = $true }
 "debug"               { $debug     = $true }
 "analysis"            { $analysis  = $true }
 "dev"                 { $dev       = $true }
 "verbose"             { $verbose   = $true }
 "cpp"                 { $force_cpp = $true }
}
}}
#endregion Argument

# Load up toolchain configuraion
. $vendor_toolchain
. $incremental_checks

$path_build = join-path $path_root  'build'
if ( -not (test-path $path_build) ) {
	new-item $path_build -ItemType Directory
}

$path_raylib_src       = join-path $path_root       'src'
$path_raylib_platforms = join-path $path_raylib_src 'platforms'
$path_raylib_glfw_inc  = join-path $path_raylib_src 'external/glfw/include'
$path_raylib_gputex    = join-path $path_raylib_src 'external/rl_gputex.h'

# Microsoft

$lib_gdi32   = 'Gdi32.lib'
$lib_shell32 = 'Shell32.lib'
$lib_xinput  = 'Xinput.lib'
$lib_user32  = 'User32.lib'
$lib_winmm   = 'Winmm.lib'

$includes = @(
	$path_raylib_src,
	$path_raylib_glfw_inc
)
foreach ($include in $includes) {
	write-host $include
}

$compiler_args = @(
	($flag_define + 'PLATFORM_DESKTOP'),
	($flag_define + 'BUILD_LIBTYPE_SHARED')
)
$linker_args   = @(
	$flag_link_dll,

	# $lib_xinput,
	$lib_gdi32,
	$lib_shell32,
	$lib_user32,
	$lib_winmm
)

if ($force_cpp) {
	$compiler_args += $flag_all_cpp
}

$raylib_headers  = Get-ChildItem -Path $path_raylib_src -Filter '*.h' -File
$raylib_modules  = get-childitem -path $path_raylib_src -filter '*.c' -file

$includes = @(
	$path_raylib_src,
	$path_raylib_glfw_inc
)
foreach ($include in $includes) {
	write-host $include
}

$dll          = join-path $path_build 'raylib.dll'
$build_result = build $path_build $includes $compiler_args $linker_args $raylib_modules $dll
