$raylib_headers  = Get-ChildItem -Path $path_raylib_src       -Filter '*.h' -File
$raylib_modules  = get-childitem -path $path_raylib_src       -filter '*.c' -file

# Refactor with refactor.exe
if ( $true ) {
	$path_refactor      = join-path $path_raylib 'raylib_c.refactor'
	$path_refactor_rlgl = join-path $path_raylib 'raylib_c_gl.refactor'

	$fmt_includes = @()
	foreach ( $header in $raylib_headers ) {
		$fmt_includes +=  split-path $header -leaf
	}
	foreach ( $module in $raylib_modules ) {
		$fmt_includes +=  split-path $module -leaf
	}

	$platform_modules = @()
	foreach ( $module in (get-childitem -path $path_raylib_platforms -filter '*.c' -file) ) {
		$platform_modules += split-path $module -leaf
	}

	$path_rlgl = join-path $path_raylib_src 'rlgl.h'

	Push-Location $path_raylib_src
		write-host "Beginning refactor...`n"
		$refactors = @(@())
		$refactorParams = @(
			# "-debug",
			"-num=$($fmt_includes.Count)"
			"-src=$($fmt_includes)",
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
		$gl_modules = @( "$path_rlgl" )

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