#region Arguments
$use_cpp   = $false
$use_snake = $false

if ( $args ) { $args | ForEach-Object {
	write-host 'arg: ' $_
switch ($_){
	"snake"	{ $use_snake = $true }
	"cpp"	{ $use_cpp   = $true }
}
}}
#endregion Arguments

$path_root     = git rev-parse --show-toplevel
$path_scripts  = $PSScriptRoot

$path_raylib_src         = join-path $path_root       'src'
$path_raylib_platforms   = join-path $path_raylib_src 'platforms'
$path_raylib_glfw_inc    = join-path $path_raylib_src 'external/glfw/include'
$path_raylib_gputex      = join-path $path_raylib_src 'external/rl_gputex.h'

write-host "Use snake: " $use_snake
write-host "Use cpp  : " $use_cpp


if ($use_cpp) {
	if ($use_snake) {
		$path_refactor      = join-path $PSScriptRoot 'raylib_cpp_snake.refactor'
		$path_refactor_rlgl = join-path $PSScriptRoot 'raylib_cpp_gl_snake.refactor'
	}
	else {
		$path_refactor      = join-path $PSScriptRoot 'raylib_cpp.refactor'
		$path_refactor_rlgl = join-path $PSScriptRoot 'raylib_cpp_gl.refactor'
	}

}
else {
	if ($use_snake) {
		$path_refactor      = join-path $PSScriptRoot 'raylib_c_snake.refactor'
		$path_refactor_rlgl = join-path $PSScriptRoot 'raylib_c_gl_snake.refactor'
	}
	else {
		$path_refactor      = join-path $PSScriptRoot 'raylib_c.refactor'
		$path_refactor_rlgl = join-path $PSScriptRoot 'raylib_c_gl.refactor'
	}
}

$raylib_headers  = Get-ChildItem -Path $path_raylib_src -Filter '*.h' -File
$raylib_modules  = get-childitem -path $path_raylib_src -filter '*.c' -file

# Refactor with refactor.exe
if ( $true ) {
	$files = @()
	foreach ( $header in $raylib_headers ) {
		$file_name = split-path $header -leaf

		if ( -not $file_name.Equals('rlgl.h' ) ) {
			$files += "$header"
		}
	}
	foreach ( $module in $raylib_modules ) {
		$files += "$module"
	}

	$files += "$path_raylib_gputex"

	$platform_modules = @()
	foreach ( $module in (get-childitem -path $path_raylib_platforms -filter '*.c' -file) ) {
		$platform_modules += "$module"
	}

	$path_rlgl = join-path $path_raylib_src 'rlgl.h'

	Push-Location $path_raylib_src
		write-host "Beginning refactor...`n"
		$refactors = @(@())
		$refactorParams = @(
			# "-debug",
			"-num=$($files.Count)"
			"-src=$($files)",
			"-spec=$($path_refactor)"
		)
		& refactor $refactorParams
		Write-Host "`nRefactoring complete`n`n"
	Pop-Location

	Push-Location $path_raylib_platforms
	write-host "Beginning refactor...`n"
		$refactors = @(@())
		$refactorParams = @(
			# "-debug",
			"-num=$($platform_modules.Count)"
			"-src=$($platform_modules)",
			"-spec=$($path_refactor)"
		)
		& refactor $refactorParams
		Write-Host "`nRefactoring complete`n`n"
	Pop-Location

	Push-Location $path_raylib_src
		$gl_modules = @( "$path_rlgl", "$path_raylib_gputex" )

		write-host "Beginning refactor just for rlgl.h...`n"
		$refactors = @(@())
		$refactorParams = @(
			# "-debug",
			"-num=$($gl_modules.Count)"
			"-src=$($gl_modules)",
			"-spec=$($path_refactor_rlgl)"
		)
		& refactor $refactorParams
		Write-Host "`nRefactoring complete`n`n"
	Pop-Location
}
