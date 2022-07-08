//-----------------------------------------------------------------------------
// Memory
//-----------------------------------------------------------------------------
const SizeU os_page_size = 4096;

function OS_Memory
os_reserve(SizeU size){
  OS_Memory result = {};
  SizeU adjusted_size = align_up(size, os_page_size);
  result.data = (U8*)VirtualAlloc(0, adjusted_size, MEM_RESERVE, PAGE_READWRITE);
  assert_msg(result.data, "Failed to reserve virtual memory");
  result.reserve = adjusted_size;
  return result;
}

function B32
os_commit(OS_Memory *m, SizeU size){
  SizeU commit = align_up(size, os_page_size);
  SizeU total_commit = m->commit + commit;
  total_commit = clamp_top(total_commit, m->reserve);
  SizeU adjusted_commit = total_commit - m->commit;
  if(adjusted_commit != 0){
    void *result = VirtualAlloc((U8*)m->data + m->commit, adjusted_commit, MEM_COMMIT, PAGE_READWRITE);
    assert_msg(result, "Failed to commit more memory");
    m->commit += adjusted_commit;
    return true;
  }
  return false;
}

function void
os_release(OS_Memory *m){
  BOOL result = VirtualFree(m->data, 0, MEM_RELEASE);
  assert_msg(result != 0, "Failed to release OS_Memory");
  if(result){
    m->data = 0;
    m->commit = 0;
    m->reserve = 0;
  }
}

function B32
os_decommit_pos(OS_Memory *m, SizeU pos){
  SizeU aligned = align_down(pos, os_page_size);
  SizeU adjusted_pos = clamp_top(aligned, m->commit);
  SizeU size_to_decommit = m->commit - adjusted_pos;
  if(size_to_decommit){
    U8 *base_address = m->data + adjusted_pos;
    BOOL result = VirtualFree(base_address, size_to_decommit, MEM_DECOMMIT);
    if(result){
      m->commit -= size_to_decommit;
      return true;
    }
  }
  return false;
}

function void
test_os_memory(){
  assert(align_down(4096, 4096) == 4096);
  assert(align_down(4095, 4096) == 0);

  OS_Memory memory = os_reserve(9000);
  assert(memory.reserve == 4096*3 && memory.data && memory.commit == 0);
  os_commit(&memory, 100);
  assert(memory.commit == 4096);
  os_commit(&memory, 100);
  assert(memory.commit == 4096*2);
  os_commit(&memory, 9000);
  assert(memory.commit == 4096*3);
  os_commit(&memory, 9000);
  assert(memory.commit == 4096*3);

  os_decommit_pos(&memory, 4096);
  assert(memory.commit == 4096);
  os_decommit_pos(&memory, 4096);
  assert(memory.commit == 4096);
  os_decommit_pos(&memory, 0);
  assert(memory.commit == 0);

  os_release(&memory);
  assert(memory.data == 0);
}

//-----------------------------------------------------------------------------
// Time
//-----------------------------------------------------------------------------
global S64 Global_counts_per_second;
api F64 os_time() {
  if (Global_counts_per_second == 0) {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    Global_counts_per_second = freq.QuadPart;
  }

  LARGE_INTEGER time;
  QueryPerformanceCounter(&time);
  F64 result = (F64)time.QuadPart / (F64)Global_counts_per_second;
  return result;
}

//-----------------------------------------------------------------------------
// Filesystem
//-----------------------------------------------------------------------------
function B32
os_write_file(String filename, String filecontent){
  FILE *f = fopen((const char *)filename.str, "w");
  if(f){
    fwrite(filecontent.str, 1, filecontent.len, f);
    fclose(f);
    return true;
  }
  return false;
}

function String
os_read_file(Allocator *a, String name){
  String result = {0};
  FILE *f = fopen((char *)name.str, "rb");
  if(f){
    fseek(f, 0, SEEK_END);
    result.len = ftell(f);
    fseek(f, 0, SEEK_SET);
    result.str = (U8 *)exp_alloc(a, result.len + 1);
    fread(result.str, result.len, 1, f);
    fclose(f);
    result.str[result.len] = 0;
  }

  return result;
}

function String
os_get_working_dir(Allocator *a){
  wchar_t buffer[2048];
  DWORD written = GetCurrentDirectoryW(2048, buffer);
  assert(written != 0);
  String16 string16 = string16_from_widechar(buffer);
  String result = string16_to_string8(a, string16);
  string_path_normalize(result);
  return result;
}

function String
os_get_exe_dir(Allocator *a){
  wchar_t buffer[2048];
  DWORD written = GetModuleFileNameW(0, buffer, 2048);
  assert(written != 0);
  String16 string16 = string16_from_widechar(buffer);
  String result = string16_to_string8(a, string16);
  string_path_normalize(result);
  result = string_chop_last_slash(result);

  if(string16.len > result.len) result.str[result.len] = 0;
  string_path_normalize(result);
  return result;
}

function String
os_get_absolute_path(Allocator *a, String path){
  Scratch scratch(a);
  String16 path16 = string8_to_string16(scratch, path);

  wchar_t *buffer = exp_alloc_array(scratch, wchar_t, 2048);
  DWORD written = GetFullPathNameW((wchar_t *)path16.str, 2048, buffer, 0);
  if(written == 0) return {};

  String16 absolute16 = string16_from_widechar(buffer);
  String   absolute   = string16_to_string8(a, absolute16);
  string_path_normalize(absolute);
  return absolute;
}

function B32
os_does_file_exist(String path){
  Scratch scratch;
  String16 path16 = string8_to_string16(scratch, path);
  DWORD attribs = GetFileAttributesW((wchar_t *)path16.str);
  B32 result = attribs == INVALID_FILE_ATTRIBUTES ? false : true;
  return result;
}

const U32 LIST_NO_FLAGS           = 0;
const U32 LIST_RECURSE_INTO_DIRS  = 1;

struct OS_File_Info{
  String relative_path;
  String absolute_path;
  B32 is_directory;
};

function Array<OS_File_Info>
os_list_dir(Allocator *a, String dir, U32 flags = LIST_NO_FLAGS){
  Scratch scratch(a);
  Array<String> dirs_to_read = {scratch};
  dirs_to_read.add(dir);

  Array<OS_File_Info> result = {a};
  for(auto it = dirs_to_read.begin(); it != dirs_to_read.end(); it++){
    String modified_path = string_fmt(scratch, "%Q\\*", it);
    String16 path16 = string8_to_string16(scratch, modified_path);

    WIN32_FIND_DATAW ffd;
    HANDLE handle = FindFirstFileW((wchar_t *)path16.str, &ffd);
    if(handle == INVALID_HANDLE_VALUE) continue;

    do{

      //
      // Skip '.' and '..'
      //
      if(ffd.cFileName[0] == '.'){
        if(ffd.cFileName[1] == '.'){
          if(ffd.cFileName[2] == 0)
            continue;
        }

        if(ffd.cFileName[1] == 0)
          continue;
      }

      String16 filename16       = string16_from_widechar(ffd.cFileName);
      String   filename         = string16_to_string8(scratch, filename16);

      String   full_file_path   = string_fmt(a, "%Q/%Q", dir, filename);
      OS_File_Info listing      = {full_file_path, os_get_absolute_path(a, full_file_path)};


      if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
        listing.is_directory = true;

        if(flags & LIST_RECURSE_INTO_DIRS){
          dirs_to_read.add(full_file_path);
        }
      }

      result.add(listing);

    }while(FindNextFileW(handle, &ffd) != 0);

    DWORD error = GetLastError();
    if(error != ERROR_NO_MORE_FILES){
      // Not sure what to do here hmmm
    }
    FindClose(handle);
  }

  return result;
}
