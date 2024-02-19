$path_root     = git rev-parse --show-toplevel
$path_scripts  = $PSScriptRoot

$path_build = join-path $path_root  'build'

if ( test-path $path_build ) {
	remove-item $path_build -recurse -Verbose
}
