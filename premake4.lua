solution 'hqtdf-win32'

projectdir = basedir()
builddir = path.join(projectdir, './build')
installdir = path.join(projectdir, './install')
bindir = path.join(installdir, 'bin')
libdir = path.join(installdir, 'lib')
includedir = path.join(installdir, 'include')

--configurations {'Debug', 'Release'}
--platforms {'x32', 'x64'}
--configurations {'Release'}
configurations {'Debug'}
platforms { 'x32'}
--platforms { 'x64'}

project 'hqtdfapi3-win32'
kind "ConsoleApp"
--kind "StaticLib"
--kind "SharedLib"

--language 'C'
language "C++"
files { "*.h", "*.cpp" }


configuration "x32"
--configuration "x64"
if os.is("windows") then
	defines '_CRT_SECURE_NO_WARNINGS'

	includedirs {"E:/LIB/boost/1.55.0/include"}
	libdirs {"E:/LIB/boost/1.55.0/lib","E:/LIB/tdfsdk/lib/Win32Debug","E:/LIB/redis-cplusplus-client/lib"}
	links{"ws2_32","anet_win32"}

	links{'TDFAPI30'}
	targetdir(bindir)
	targetname('hqtdf-win32')
elseif os.is("linux") then
	includedirs { "/usr/local/include" }
	libdirs { "/usr/local/lib" }
	links { "lua","expat" }
end

configuration "Debug"
defines { "DEBUG" }
flags { "Symbols" }

--configuration "Release"
--defines { "NDEBUG" }
--flags { "Optimize" }




