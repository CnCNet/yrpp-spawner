#ifndef VERSION_H
#define VERSION_H

#define _WSTR(x) _WSTR_(x)
#define _WSTR_(x) L ## #x
#define _STR(x) _STR_(x)
#define _STR_(x) #x

// Indicates project maturity and completeness
#define VERSION_MAJOR 0
// Indicates major changes and significant additions, like new logics
#define VERSION_MINOR 0
// Indicates minor changes, like vanilla bugfixes, unhardcodings or hacks
#define VERSION_REVISION 0
// Indicates YRpp-Spawner-related bugfixes only
#define VERSION_PATCH 16

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
	#define FILE_VERSION_STR _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "." _STR(VERSION_REVISION) "." _STR(VERSION_PATCH)
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH
	#define PRODUCT_VERSION "Release Build " FILE_VERSION_STR
#elif defined(GIT_BRANCH) // Nightly devbuild metadata

	#define FILE_VERSION_STR "Commit " STR_GIT_COMMIT
	#define FILE_VERSION 0,0,0,0
	#define PRODUCT_VERSION "Nightly Build " STR_GIT_COMMIT " @ " STR_GIT_BRANCH
#else // Regular devbuild metadata
	#define FILE_VERSION_STR "Commit " STR_GIT_COMMIT
	#define FILE_VERSION 0,0,0,0
	#define PRODUCT_VERSION "Development Build " STR_GIT_COMMIT
#endif

#endif // VERSION_H
