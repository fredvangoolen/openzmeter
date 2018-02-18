/**
 * @package de.atwillys.cc.swl
 * @license BSD (simplified)
 * @author Stefan Wilhelm (stfwi)
 *
 * @file resource_section.hh
 * @ccflags
 * @ldflags
 * @platform linux, bsd, !windows
 * @standard >= c++98
 *
 * -----------------------------------------------------------------------------
 *
 * Allows to embed and access resource files in the compiled binary on Linux/UNIX
 * platforms. The data are stored as single USTAR tape archive in a section with
 * configurable name (default: .blobres), which is not allocated in the RAM on
 * program startup. The class template `sw::resource_section` can be used to
 * list, load (allocate) and extract (to file) the information stored in the
 * section.
 *
 * C++ code usage:
 *
 *  #include <sw/resource_section.hh>
 *  #include <iostream>
 *  #include <string>
 *
 *  using namespace std;
 *
 *  // Initialises the file list implicitly.
 *  sw::resource_section resources;
 *
 *  int main()
 *  {
 *    // Example key, corresponds to a file path in the tar'ed directory.
 *    string key = "path/in/tar/file.txt";
 *
 *    // Just a possibility to double check, as the initialisation does not throw:
 *    if(!resources.valid()) {
 *      cerr << "Resources failed to initialise!" << endl;
 *      return 1;
 *    }
 *
 *    // Check if a resource exists (does not throw)
 *    if(!resources.has(key)) {
 *      cerr << "Resource not found:" << key << endl;
 *    }
 *
 *    // Access a binary large object (does not throw, always returns a valid reference,
 *    // even if the key is not found - then a static const dummy instance is returned).
 *    const sw::resource_section::blob & blob = resources[key];
 *
 *    // Get the name (==key) of the resource
 *    if(blob.path() != key) { cerr << "Weired bug in resource lib" << endl; return 1;  }
 *
 *    // Get the offset (file seek position) in this binary where the tar header block
 *    // of this resource can be found. The data starts 512 bytes after that header.
 *    // If the resource is not found, the offset() is 0.
 *    sw::resource_section::position_type header_offset = blob.offset();
 *    sw::resource_section::position_type data_offset = blob.offset() + 512;
 *
 *    // Get the length of the embedded file. The length does NOT include the tar header,
 *    // only the "payload" file data size. If the resource is not found/invalid, the size()
 *    // is 0.
 *    sw::resource_section::size_type sz = blob.size();
 *
 *    // Load a resource (does not throw, except on exceptional STL container errors maybe)
 *    if(!resources.load(key)) {
 *      cerr << "Error loading resource: " << key << endl;
 *      return 1;
 *    }
 *
 *    // Accessing allocated (loaded) data (returns valid pointer, not NULL)
 *    const void *data = blob.data();
 *    cout.write((const char*)data, sz);
 *
 *    // Freeing the memory of a blob data. This does not clear the name, offset and size.
 *    // only the data.
 *    blob.clear();
 *
 *    // Extract to external file
 *    if(!resources.extract(key, "/tmp/resource_test_extraction.tmp")) {
 *      cerr << "Error extracting resource: " << key << endl;
 *      return 1;
 *    }
 *
 *    // Free all resource data buffers (not required due to the use of STL containers),
 *    // But this does not clear the index map.
 *    resources.clear();
 *
 *    return 0;
 *  }
 *
 * -----------------------------------------------------------------------------
 *
 * Making the resource section:
 *
 *  #
 *  # Compile main.cc
 *  #
 * 	g++ -c $(CCFLAGS) -o main.o main.cc $(INCLUDE_PATH)
 *
 *  #
 *  # Make USTAR blobres.tar
 *  #
 * 	tar --group=nogroup --owner=nobody --dereference --recursion \
 *    --create --format=ustar --directory=$(MY_RESOURCE_DIRECTORY) -f blobres.tar .
 *
 *  #
 *  # Make object from tar
 *  # NOTE: You must specify your elf type correctly, for 64bit PCs, it is "elf64-x86-64"
 *  # and "i386".
 *  #
 *	objcopy --input binary --output elf64-x86-64 --binary-architecture i386 --rename-section \
 *	  .data=.blobres,load,readonly,data,contents blobres.tar blobres.o
 *
 *  #
 *  # Tar not needed anymore
 *  #
 *  rm -f blobres.tar
 *
 *  #
 *  # Link the objects
 *  #
 *	gcc -o app blobres.o main.o -lstdc++
 *
 *  # Run programm
 *  ./app
 *
 * -----------------------------------------------------------------------------
 *
 * Notes:
 *
 *  - The resource_section objects initialises during construction, indexing the embedded tar.
 *
 *  - The resource_section objects are not thread synchronised, means if you have a global
 *    resource object and load/access data in more than one thread, then you have implement
 *    a locking mechanism. This does not apply when you instantiate an object for each thread.
 *    For thread safe loading use `bool load(const str_t & key, void *where, size_type size)`.
 *
 *  - If the executable is not readable, the initialisation will fail, and valid() will be false.
 *
 *  - Generally the use of exceptions is avoided in this class. Only STL containers may throw
 *    when memory allocation fails (or stack variables allocation errors). The `load()` method
 *    internally catches container allocation errors and returns false.
 *
 *  - The integrity of the USTAR tar is not checked. That should be done before compiling/linking.
 *    Means: The tar type must be USTAR (`tar --format=ustar`), and the block header checksums
 *    are not validated. Only the block magick entry is checked. As the tar is USTAR, the file
 *    sizes are restricted to 8GB.
 *
 *  - You can define an own section name for the "BLOB" section using `-DRESOURCE_SECTION_NAME=...`
 *    (default is: blobres), and you can strip characters from the beginning of the tar file paths
 *    with -DRESOURCE_TAR_NAME_STRIP=... Default is "./", which is the case when typing
 *    `tar [other options] --format=ustar -cf outfile.tar .` (tar the current directory, useful
 *    with the `--directory=` option).
 *
 * -----------------------------------------------------------------------------
 * +++ BSD license header +++
 * Copyright (c) 2008-2014, Stefan Wilhelm (stfwi, <cerbero s@atwilly s.de>)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met: (1) Redistributions
 * of source code must retain the above copyright notice, this list of conditions
 * and the following disclaimer. (2) Redistributions in binary form must reproduce
 * the above copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the distribution.
 * (3) Neither the name of atwillys.de nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS
 * AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * -----------------------------------------------------------------------------
 */
#ifndef SW_RESOURCE_SECTION_HH
#define	SW_RESOURCE_SECTION_HH

// <editor-fold desc="preprocessor" defaultstate="collapsed">

/* Default resource blob section name */
#ifndef RESOURCE_SECTION_NAME
#define RESOURCE_SECTION_NAME blobres
#endif

/* Default file name correction: Remove leading "./" */
#ifndef RESOURCE_TAR_NAME_STRIP
#define RESOURCE_TAR_NAME_STRIP "./"
#endif

#if defined (__linux__) || defined (__linux)
  #include <linux/elf.h>
#elif defined(unix) || defined(__unix__) || defined(__unix) || defined(__FreeBSD__) || \
      defined(__APPLE__) || defined(__MACH__) || defined(__OpenBSD__) || defined(__NetBSD__) || \
      defined(__DragonFly__)
  #include <elf.h>
  #if defined (__FreeBSD__) || defined(__OpenBSD__)
    #include <sys/sysctl.h>
  #endif
#else
  #error "BLOB resource section storage not supported on this platform"
#endif

#include <string>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include <limits.h>
// </editor-fold>

// <editor-fold desc="basic_blob_section" defaultstate="collapsed">

namespace sw { namespace detail {

template <typename Position_Type=uint64_t, typename Size_Type=uint64_t>
class basic_resource_section
{
public:

  class blob;
  typedef Position_Type position_type;
  typedef Size_Type size_type;
  typedef char char_t;
  typedef std::string str_t;
  typedef std::map<str_t, blob> blob_map_t;

  /**
   * Subclass blob, used for the individual resource entries
   */
  class blob
  {
    friend class basic_resource_section;

  public:

    /**
     * Standard constructor
     */
    inline blob() : path_(), offset_(0), size_(0), data_()
    { ; }

    /**
     * Standard constructor
     */
    inline blob(const str_t &path, position_type offs, size_type sz) : path_(path),
      offset_(offs), size_(sz), data_()
    { ; }

    /**
     * Copy constructor.
     * @param const blob& o
     */
    inline blob(const blob& o) : path_(o.path_), offset_(o.offset_), size_(o.size_), data_(o.data_)
    { ; }

    /**
     * Assignment.
     * @param const blob& o
     */
    inline blob& operator= (const blob& o)
    { path_=(o.path_); offset_=(o.offset_); size_=(o.size_); data_=(o.data_); return *this; }

    /**
     * Destructor
     */
    virtual ~blob()
    { ; }

  public:

    /**
     * Returns if this instance is valid. All instances in the index map are,
     * but the basic_resource_section::_no_blob instance is not.
     * @return bool
     */
    inline bool valid() const throw()
    { return offset_ != 0; }

    /**
     * Returns the name of the resource file. Empty string if not valid.
     * @return const str_t
     */
    inline const str_t & path() const throw()
    { return path_; }

    /**
     * Returns the offset of the resource BLOB in this file
     * @return position_type
     */
    inline position_type offset() const throw()
    { return offset_; }

    /**
     * Returns the size of the resource BLOB
     * @return size_type
     */
    inline size_type size() const throw()
    { return size_; }

    /**
     * Returns pointer to the data buffer if the resource was loaded.
     * This data pointer is (or should be) never NULL when the object is constructed.
     * @return const void *
     */
    inline const void * data() const throw()
    { return (void*) data_.data(); }

    /**
     * Returns the data as string, a reference to an empty string of not loaded.
     * @return const str_t
     */
    inline const str_t & data_str() const throw()
    { return data_; }

    /**
     * Returns true if the resource data is present in memory
     * @return bool
     */
    inline bool loaded() const throw()
    { return (offset_ != 0) && ((size_==0) || (data_.length()>0)); }

    /**
     * Unloads the resource. Name, size and offset remain unmodified.
     */
    inline void clear()
    { data_.clear(); str_t s; s.swap(data_); }

  private:

    str_t path_;
    position_type offset_;
    size_type size_;
    str_t data_;
  };

public:

  /**
   * Constructor
   */
  inline basic_resource_section() : position_(0), size_(0), blobs_(), valid_(false)
  { initialize(); }

  /**
   * Destructor
   */
  virtual ~basic_resource_section()
  { ; }

private:

  /** No copying */
  inline basic_resource_section(const basic_resource_section & o)
  { ; }

  /** No copying */
  basic_resource_section & operator = (const basic_resource_section & o)
  { ; }

public:

  /**
   * Returns true if all initialisation steps succeeded and the instance is usable.
   * @return bool
   */
  inline bool valid() const throw()
  { return valid_; }

  /**
   * Returns true if a resource defined by `key` exists.
   * @param const str_t & key
   * @return bool
   */
  inline bool has(const str_t & key) const throw()
  { return blobs_.find(key) != blobs_.end(); }

  /**
   * Returns the path to the currently running application
   * @return const str_t &
   */
  inline const str_t & application_path() const throw()
  { return app_path_; }

  /**
   * Returns the offset of the resource blob section in this binary
   * @return position_type
   */
  inline position_type section_offset() const throw()
  { return position_; }

  /**
   * Returns the size of the resource blob section in this binary
   * @return size_type
   */
  inline size_type section_size() const throw()
  { return size_; }

  /**
   * Returns the index map of the resource tar. Keys are the file names, values are
   * blob objects.
   * @return const blob_map_t &
   */
  inline const blob_map_t & blobs() const throw()
  { return blobs_; }

public:

  // Map wrappers for better accessibility.
  /**
   * blob_map_t Iterator.
   */
  typedef typename blob_map_t::iterator iterator;
  typedef typename blob_map_t::const_iterator const_iterator;

  /**
   * Returns the begin of the blob map.
   * @return const_iterator
   */
  inline const_iterator begin() const
  { return blobs_.begin(); }

  /**
   * Returns the begin of the blob map.
   * @return iterator
   */
  inline iterator begin()
  { return blobs_.begin(); }

  /**
   * Returns the end of the blob map.
   * @return const_iterator
   */
  inline const_iterator end() const
  { return blobs_.end(); }

  /**
   * Returns the end of the blob map.
   * @return iterator
   */
  inline iterator end()
  { return blobs_.end(); }

  /**
   * Returns the resource blob object of the key `key` or a reference to a "not found"
   * instance
   * @param const str_t & key
   * @return const blob &
   */
  inline const blob & operator[] (const str_t & key)
  { const_iterator it = blobs_.find(key); return (it != blobs_.end()) ? it->second : no_blob_; }

public:

    /**
     * Free memory of the allocated (loaded) blobs, but not the index and the blob objects
     * themselves (name, size, and offset will be preserved).
     */
    void clear()
    { for(iterator it = blobs_.begin(); it != blobs_.end(); ++it) it->second.clear(); }

    /**
     * Loads a resource into the instance data buffer (allocates). Only throws on unexpected
     * allocation errors.
     * @return bool
     */
    bool load(const str_t & key)
    {
      FILE *fp; // C style access without exceptions
      size_type len;
      iterator it = blobs_.find(key);
      if(it == blobs_.end()) return false;
      if(!it->second.offset() || it->second.path().empty()) return false;
      if(!it->second.size() || it->second.data_.length() > 0) return true;
      try { it->second.data_.reserve(size_); } catch(...) { return false; }
      #define clret() { fclose(fp); str_t s; s.swap(it->second.data_); return false; }
      #define BLOCKSZ sizeof(tar_ustar_t)
      if(!(fp = fopen(application_path().c_str(), "rb"))) return false;
      if(fseek(fp, it->second.offset()+BLOCKSZ, SEEK_SET)!=0) clret();
      len = it->second.size();
      size_type pos = 0;
      char buf[BLOCKSZ];
      while(len > 0) {
        size_t n = fread(buf, 1, len < BLOCKSZ ? len : BLOCKSZ, fp);
        if(!n) clret();
        for(register size_t i=0; i<n; ++i,++pos) it->second.data_ += buf[i]; // improve that
        len -= n;
      }
      fclose(fp);
      #undef BLOCKSZ
      #undef clret
      return true;
    }

    /**
     * Loads a resource into a defined buffer. Only throws on unexpected allocation errors.
     * @return bool
     */
    bool load(const str_t & key, void *where, size_type size)
    {
      iterator it = blobs_.find(key);
      if(it == blobs_.end()) return false;
      if(!it->second.offset() || it->second.path().empty()) return false;
      if(!where || !size || size < it->second.size()) return false;
      #define clret() { fclose(fp); return false; }
      #define BLOCKSZ sizeof(tar_ustar_t)
      FILE *fp;
      if(!(fp = fopen(application_path().c_str(), "rb"))) return false;
      if(fseek(fp, it->second.offset()+BLOCKSZ, SEEK_SET)!=0) clret();
      size_type len = it->second.size();
      register char *pos = (char*) where;
      char buf[BLOCKSZ];
      while(len > 0) {
        size_t n = fread(buf, 1, len < BLOCKSZ ? len : BLOCKSZ, fp);
        if(!n) clret();
        for(register size_t i=0; i<n; ++i,++pos) *pos = buf[i];
        len -= n;
      }
      fclose(fp);
      #undef BLOCKSZ
      #undef clret
      return true;
    }


    /**
     * Extracts the resource into a (file system) file. Does only throw on c++ allocation errors.
     * @return bool
     */
    bool extract(const str_t & key, const str_t & file)
    {
      FILE *fp, *ofp;
      size_type len;
      iterator it = blobs_.find(key);
      if(it == blobs_.end()) return false;
      if(!it->second.offset() || it->second.path().empty()) return false;
      if(!it->second.size() || it->second.data_.length() > 0) return true;
      #define clret() { fclose(fp); fclose(ofp); return false; }
      #define BLOCKSZ sizeof(tar_ustar_t)
      if(!(fp = fopen(application_path().c_str(), "rb"))) return false;
      if(!(ofp = fopen(file.c_str(), "wb"))) { fclose(fp); return false; }
      if(fseek(fp, it->second.offset()+BLOCKSZ, SEEK_SET)!=0) clret();
      len = it->second.size();
      char buf[BLOCKSZ];
      while(len > 0) {
        size_t n = fread(buf, 1, len < BLOCKSZ ? len : BLOCKSZ, fp);
        if(!n) clret();
        if(fwrite(buf, 1, n, ofp) != n) clret();
        len -= n;
      }
      fclose(fp);
      fclose(ofp);
      #undef BLOCKSZ
      #undef clret
      return true;
    }

public:

  /**
   * Determines the resource blob section file offset and size, as well as the file system path
   * to this program. `app_path` is a pointer to a readonly (write-once) static function variable,
   * the content is never set to NULL.
   *
   * Returns 0 on success, nonzero on failure.
   *
   * Note: Call this function once before starting threads to initialise write-once
   *       static variables.
   *
   * Note: This function can be directly extracted to a C programs defining position_type
   *       and size_type == uint64_t. It is written in C style.
   *
   * @param position_type *position
   * @param size_type *size
   * @param const char **app_path
   * @return int
   */
  static int resource_section(position_type *position, size_type *size, const char **app_path) throw()
  {
    /* ELF arch selection */
    #if (defined __IA64__ || defined __ia64__ || defined __x86_64__)
    #define Ehdr Elf64_Ehdr
    #define Shdr Elf64_Shdr
    #define clsid ELFCLASS64
    #else
    #define Ehdr Elf32_Ehdr
    #define Shdr Elf32_Shdr
    #define clsid ELFCLASS32
    #endif
    #define clret() { fclose(fp); return -1; }
    #define seek_read(OUT, SIZE, POS) \
      if((fseek(fp, (POS), SEEK_SET)) || (fread(OUT, 1, (SIZE), fp) != (SIZE))) clret();

    /* Determine data */
    FILE *fp;
    char bf[64];
    Ehdr hd;
    Shdr sh;
    int64_t i, stab_adr;
    static char _app_path[PATH_MAX+1];

    if(position) *position = 0;
    if(size) *size = 0;
    if(app_path) *app_path = _app_path;

    do {
      memset(_app_path, 0, sizeof(_app_path));
      #if defined (__linux__)
      strncpy(_app_path, "/proc/self/exe", sizeof(_app_path)-1);
      #elif defined (__NetBSD__)
      strncpy(_app_path, "/proc/curproc/exe", sizeof(_app_path)-1);
      #elif defined (__FreeBSD__) || defined (__OpenBSD__)
      int ic[4];
      ic[0] = CTL_KERN; ic[1] = KERN_PROC; ic[2] = KERN_PROC_PATHNAME; ic[3] = -1;
      size_t sz = sizeof(_app_path)-1;
      if(sysctl(ic, 4, _app_path, &sz, NULL, 0)) _app_path[0] = '\0';
      #elif defined (__DragonFly__)
      strncpy(_app_path, "/proc/curproc/file", sizeof(_app_path)-1);
      #elif defined (__APPLE__) && __MACH__
      uint32_t sz = sizeof(_app_path);
      if(_NSGetExecutablePath(_app_path, &sz)) _app_path[0] = '\0';
      #endif
    } while(0);

    if(!(fp = fopen(_app_path, "rb"))) return -1;
    seek_read(bf, EI_NIDENT, 0);
    if(strncmp(ELFMAG, bf, 4) || (!bf[EI_CLASS] || bf[EI_CLASS] > 2)) clret();
    if(bf[EI_CLASS] != clsid) { fclose(fp); return -2; } // Special error: Incorrect compilation
    seek_read(&hd, sizeof(Ehdr), 0);
    if(hd.e_shentsize != sizeof(Shdr)) clret();
    seek_read(&sh, sizeof(Shdr), hd.e_shoff + (hd.e_shstrndx*sizeof(Shdr)));
    stab_adr = sh.sh_offset;
    /* Iterate sections, check flags, type and name, conditionally set offset return 0==OK */
    for(i = hd.e_shnum-1; i >= 0; --i) {
      seek_read(&sh, sizeof(Shdr), hd.e_shoff + (i*sizeof(Shdr)));
      if(sh.sh_size && (sh.sh_type==SHT_PROGBITS) && !((sh.sh_flags & SHF_MASKPROC) & (~SHF_WRITE))) {
        memset(bf, 0, sizeof(bf));
        seek_read(bf, sizeof(bf)-1, stab_adr + sh.sh_name); /* tab is not at eof and strings 0 term */
        #define defined_strs(X) "." #X
        #define defined_str(X) defined_strs(X)
        if(strncmp(defined_str(RESOURCE_SECTION_NAME), bf, sizeof(bf)) == 0) {
          /* Section found */
          if(position) *position = sh.sh_offset;
          if(size) *size = sh.sh_size;
          fclose(fp);
          return 0;
        }
        #undef defined_str
        #undef defined_strs
      }
    }
    clret();
    /* Prepr. cleanup */
    #undef seek_read
    #undef Ehdr
    #undef Shdr
    #undef clsid
    #undef clret
    return -666; /* Only called if there is a bug */
  }

protected:

  /**
   * Initialises the object. No throw, except when adding the index to the _blobs map fails.
   * @return int
   */
  int initialize()
  {
    #define clret(E) { fclose(fp); return (E); }

    // Get section info
    FILE *fp;
    const char *app_path;
    position_type offset;
    size_type size;
    tar_ustar_t tar;
    position_type read_position;
    position_type end_position;
    char file_name[101];
    size_type file_size = 0;
    const position_type BSZ = sizeof(tar_ustar_t);

    blobs_.clear();
    if(resource_section(&offset, &size, &app_path) != 0) return -1;
    app_path_ = app_path;
    position_ = offset;
    size_ = size;

    // Tar index
    end_position = offset + (position_type)size;
    if(!(fp = fopen(app_path, "rb"))) return -1;
    if((fseek(fp, offset, SEEK_SET))) clret(-2);
    read_position = offset;
    while(read_position < end_position) {
      if(fread(&tar, 1, BSZ, fp) != BSZ) clret(-3);
      if(strncmp(tar.magic, "ustar", 5) != 0) {
        int i;
        if(read_position == offset) clret(-4);
        for(i=0; i<(512/4); ++i) if(((uint32_t*)&tar)[i] != 0) clret(-5); // Check tar EOF
        break;
      }
      // Block check, only regular files are used.
      if(strncmp(tar.version, "00", 2) != 0) clret(-6);
      if(tar.typeflag != '0') {
        file_size = 0;
      } else {
        do { // Name
          strncpy(file_name, tar.name, sizeof(file_name)-1);
          file_name[sizeof(file_name)-1] = '\0';
          if(file_name[0] == '\0') clret(-7);
        } while(0);
        int i;
        for(file_size=0, i=0; i<11; ++i) { // Size: 11 octal bits + space
          if(tar.size[i]<'0' || tar.size[i]>'7') clret(-8);
          file_size <<= 3;
          file_size += tar.size[i] - '0';
        }
        do {
          str_t s = file_name;
          if(s.find(RESOURCE_TAR_NAME_STRIP) == 0) {
            if(s.length() <= strlen(RESOURCE_TAR_NAME_STRIP)) break;
            s = s.substr(strlen(RESOURCE_TAR_NAME_STRIP));
          }
          blob b(s, read_position, file_size);
          blobs_[s] = b;
        } while(0);
      }
      read_position += BSZ + ((file_size==0) ? 0 : (((file_size+BSZ-1)/BSZ) * BSZ));
      if((::fseek(fp, read_position, SEEK_SET))) clret(-9);
    }
    fclose(fp);
    valid_ = true;
    #undef clret
    return 0;
  }

protected:

  /**
   * Auxiliary structure for STAR tape archive format
   */
  typedef struct { char
    name[100], mode[8], uid[8], gid[8], size[12], mtime[12], chksum[8], typeflag, linkname[100],
    magic[6], version[2], uname[32], gname[32], devmajor[8], devminor[8], prefix[155], z_resrv[12];
  } tar_ustar_t;

  // Instance variables
  str_t app_path_;
  position_type position_;
  size_type size_;
  blob_map_t blobs_;
  bool valid_;
  static const blob no_blob_;
};

template <typename P, typename SZ>
const typename basic_resource_section<P,SZ>::blob basic_resource_section<P,SZ>::no_blob_;

}}
// </editor-fold>

// <editor-fold desc="specialisation" defaultstate="collapsed">
namespace sw {
  typedef detail::basic_resource_section<> resource_section;
}
// </editor-fold>

#endif

