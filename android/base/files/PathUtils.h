// Copyright 2014 The Android Open Source Project
//
// This software is licensed under the terms of the GNU General Public
// License version 2, as published by the Free Software Foundation, and
// may be copied, distributed, and modified under those terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#pragma once

#include "android/base/containers/StringVector.h"
#include "android/base/String.h"
#include "android/base/StringView.h"

namespace android {
namespace base {

// Utility functions to manage file paths. None of these should touch the
// file system. All methods must be static.
class PathUtils {
public:
    // An enum listing the supported host file system types.
    // HOST_POSIX means a Posix-like file system.
    // HOST_WIN32 means a Windows-like file system.
    // HOST_TYPE means the current host type (one of the above).
    // NOTE: If you update this list, modify kHostTypeCount below too.
    enum HostType {
        HOST_POSIX = 0,
        HOST_WIN32 = 1,
#ifdef _WIN32
        HOST_TYPE = HOST_WIN32,
#else
        HOST_TYPE = HOST_POSIX,
#endif
    };

    // The number of distinct items in the HostType enumeration above.
    static const int kHostTypeCount = 2;

    // Suffixes for an executable file (.exe on Windows, empty otherwise)
    static const char* const kExeNameSuffixes[kHostTypeCount];

    // Suffixe for an executable file on the current platform
    static const char* const kExeNameSuffix;

    // Returns the executable name for a base name |baseName|
    static String toExecutableName(StringView baseName,
                                   HostType hostType);

    static String toExecutableName(StringView baseName) {
        return toExecutableName(baseName, HOST_TYPE);
    }

    // Return true if |ch| is a directory separator for a given |hostType|.
    static bool isDirSeparator(int ch, HostType hostType);

    // Return true if |ch| is a directory separator for the current platform.
    static bool isDirSeparator(int ch) {
        return isDirSeparator(ch, HOST_TYPE);
    }

    // Return true if |ch| is a path separator for a given |hostType|.
    static bool isPathSeparator(int ch, HostType hostType);

    // Return true if |ch| is a path separator for the current platform.
    static bool isPathSeparator(int ch) {
        return isPathSeparator(ch, HOST_TYPE);
    }

    // Return the directory separator character for a given |hostType|
    static char getDirSeparator(HostType hostType) {
        return (hostType == HOST_WIN32) ? '\\' : '/';
    }

    // Remove trailing separators from a |path| string, for a given |hostType|.
    static String removeTrailingDirSeparator(StringView path,
                                             HostType hostType);

    // Remove trailing separators from a |path| string for the current host.
    static String removeTrailingDirSeparator(StringView path) {
        return removeTrailingDirSeparator(path, HOST_TYPE);
    }

    // Add a trailing separator if needed.
    static String addTrailingDirSeparator(StringView path,
                                          HostType hostType);

    // Add a trailing separator if needed.
    static String addTrailingDirSeparator(StringView path) {
        return addTrailingDirSeparator(path, HOST_TYPE);
    }

    // If |path} starts with a root prefix, return its size in bytes, or
    // 0 otherwise. The definition of valid root prefixes depends on the
    // value of |hostType|. For HOST_POSIX, it's any path that begins
    // with a slash (/). For HOST_WIN32, the following prefixes are
    // recognized:
    //    <drive>:
    //    <drive>:<sep>
    //    <sep><sep>volumeName<sep>
    static size_t rootPrefixSize(StringView path, HostType hostType);

    // Return the root prefix for the current platform. See above for
    // documentation.
    static size_t rootPrefixSize(StringView path) {
        return rootPrefixSize(path, HOST_TYPE);
    }

    // Return true iff |path| is an absolute path for a given |hostType|.
    static bool isAbsolute(StringView path, HostType hostType);

    // Return true iff |path| is an absolute path for the current host.
    static bool isAbsolute(StringView path) {
        return isAbsolute(path, HOST_TYPE);
    }

    // Return an extension part of the name/path (the part of the name after
    // last dot, including the dot. E.g.:
    //  "file.name" -> ".name"
    //  "file" -> ""
    //  "file." -> "."
    //  "/full/path.png" -> ".png"
    static StringView extension(StringView path, HostType hostType = HOST_TYPE);

    // Split |path| into a directory name and a file name. |dirName| and
    // |baseName| are optional pointers to strings that will receive the
    // corresponding components on success. |hostType| is a host type.
    // Return true on success, or false on failure.
    // Note that unlike the Unix 'basename' command, the command will fail
    // if |path| ends with directory separator or is a single root prefix.
    // Windows root prefixes are fully supported, which means the following:
    //
    //     /            -> error.
    //     /foo         -> '/' + 'foo'
    //     foo          -> '.' + 'foo'
    //     <drive>:     -> error.
    //     <drive>:foo  -> '<drive>:' + 'foo'
    //     <drive>:\foo -> '<drive>:\' + 'foo'
    //
    static bool split(StringView path,
                      HostType hostType,
                      String* dirName,
                      String* baseName);

    // A variant of split() for the current process' host type.
    static bool split(StringView path,
                             String* dirName,
                             String* baseName) {
        return split(path, HOST_TYPE, dirName, baseName);
    }

    // Join two path components together. Note that if |path2| is an
    // absolute path, this function returns a copy of |path2|, otherwise
    // the result will be the concatenation of |path1| and |path2|, if
    // |path1| doesn't end with a directory separator, a |hostType| specific
    // one will be inserted between the two paths in the result.
    static String join(StringView path1, StringView path2, HostType hostType);

    // A variant of join() for the current process' host type.
    static String join(StringView path1, StringView path2) {
        return join(path1, path2, HOST_TYPE);
    }

    // A convenience function to join a bunch of paths at once
    template <class... Paths>
    static String join(StringView path1, StringView path2, Paths&&... paths) {
        return join(path1, join(path2, std::forward<Paths>(paths)...));
    }

    // Decompose |path| into individual components. If |path| has a root
    // prefix, it will always be the first component. I.e. for Posix
    // systems this will be '/' (for absolute paths). For Win32 systems,
    // it could be 'C:" (for a path relative to a root volume) or "C:\"
    // for an absolute path from volume C).,
    // On success, return true and sets |out| to a vector of strings,
    // each one being a path component (prefix or subdirectory or file
    // name). Directory separators do not appear in components, except
    // for the root prefix, if any.
    static StringVector decompose(StringView path, HostType hostType);

    // Decompose |path| into individual components for the host platform.
    // See comments above for more details.
    static StringVector decompose(StringView path) {
        return decompose(path, HOST_TYPE);
    }

    // Recompose a path from individual components into a file path string.
    // |components| is a vector of strings, and |hostType| the target
    // host type to use. Return a new file path string. Note that if the
    // first component is a root prefix, it will be kept as is, i.e.:
    //   [ 'C:', 'foo' ] -> 'C:foo' on Win32, but not Posix where it will
    // be 'C:/foo'.
    static String recompose(const StringVector& components,
                                 HostType hostType);

    // Recompose a path from individual components into a file path string
    // for the current host. |components| is a vector os strings.
    // Returns a new file path string.
    static String recompose(const StringVector& components) {
        return recompose(components, HOST_TYPE);
    }

    // Given a list of components returned by decompose(), simplify it
    // by removing instances of '.' and '..' when that makes sense.
    // Note that it is not possible to simplify initial instances of
    // '..', i.e. "foo/../../bar" -> "../bar"
    static void simplifyComponents(StringVector* components);
};

// Useful shortcuts to avoid too much typing.
static const PathUtils::HostType kHostPosix = PathUtils::HOST_POSIX;
static const PathUtils::HostType kHostWin32 = PathUtils::HOST_WIN32;
static const PathUtils::HostType kHostType = PathUtils::HOST_TYPE;

}  // namespace base
}  // namespace android
