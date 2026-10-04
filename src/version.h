#ifndef VERSION_H
#define VERSION_H

#define _WSTR(x) _WSTR_(x)
#define _WSTR_(x) L ## #x
#define _STR(x) _STR_(x)
#define _STR_(x) #x

#if defined(IS_CNCNET_YR_VER) && defined(IS_HARDENED_VER)
	#define PRODUCT_TYPE "(CnCNet YR, hardened)"
#elif defined(IS_CNCNET_YR_VER)
	#define PRODUCT_TYPE "(CnCNet YR)"
#elif defined(IS_HARDENED_VER)
	#define PRODUCT_TYPE "(hardened)"
#else
	#define PRODUCT_TYPE "(regular)"
#endif

#define PRODUCT_NAME "YRpp Spawner " PRODUCT_TYPE
#define FILE_DESCRIPTION "CnCNet Yuri's Revenge Spawner for Syringe, based on YRpp " PRODUCT_TYPE

// GitCommit is supplied by CI or resolved from Git by MSBuild.
#ifndef GIT_COMMIT
	#define GIT_COMMIT unknown
#endif

#define STR_GIT_COMMIT _STR(GIT_COMMIT)
#ifdef GIT_BRANCH
	#define STR_GIT_BRANCH _STR(GIT_BRANCH)
#endif

#ifdef IS_RELEASE_VER // Release build metadata
	#if defined(VERSION_MAJOR) && defined(VERSION_MINOR) && defined(VERSION_REVISION) && defined(VERSION_PATCH)
		#define FILE_VERSION_STR _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "." _STR(VERSION_REVISION) "." _STR(VERSION_PATCH)
		#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH
	#else // Local Release builds use a placeholder unless ReleaseVersion is supplied explicitly.
		#define FILE_VERSION_STR "x.x.x.x"
		#define FILE_VERSION 0,0,0,0
	#endif
	#define PRODUCT_VERSION "Release Build " FILE_VERSION_STR
#elif defined(IS_NIGHTLY_VER) // Nightly build metadata
	#define FILE_VERSION_STR "Commit " STR_GIT_COMMIT
	#define FILE_VERSION 0,0,0,0
	#ifdef GIT_BRANCH
		#define PRODUCT_VERSION "Nightly Build " STR_GIT_COMMIT " @ " STR_GIT_BRANCH
	#else // Local Nightly builds show the commit ID without a CI-supplied branch.
		#define PRODUCT_VERSION "Nightly Build " STR_GIT_COMMIT
	#endif
#else // Local Debug builds keep the commit ID in FileVersion; ProductVersion is simply "Debug Build ".
	#define FILE_VERSION_STR "Commit " STR_GIT_COMMIT
	#define FILE_VERSION 0,0,0,0
	#define PRODUCT_VERSION "Debug Build "
#endif

#endif // VERSION_H
