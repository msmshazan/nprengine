
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <shellscalingapi.h>
#undef NOMINMAX
#include <intrin.h>
#include <stdint.h>
#include <bgfx/c99/bgfx.h>
#include <meshoptimizer.h>
#define ZPL_IMPLEMENTATION
#include "zpl.h"
#include "lodepng.cpp"
#include "imgui.cpp"
#include "imgui_widgets.cpp"
#include "imgui_draw.cpp"
#include "imgui_demo.cpp"
#include "imgui_impl_win32.cpp"

typedef struct
Rect
{
    int X;
    int Y;
    int Width;
    int Height;
}
Rect;

typedef struct
DrawCommand
{
    int64_t id;
}
DrawCommand;

typedef struct
Buffer
{
    void* data;
    size_t size;
}
Buffer;

typedef struct
Aabb
{
    zpl_vec3 min;
    zpl_vec3 max;
}
Aabb;

typedef struct
Capsule
{
    zpl_vec3 pos;
    zpl_vec3 end;
    float radius;
}
Capsule;

typedef struct
Cone
{
    zpl_vec3 pos;
    zpl_vec3 end;
    float radius;
}
Cone;


typedef struct
Cylinder
{
    zpl_vec3 pos;
    zpl_vec3 end;
    float radius;
}
Cylinder;


typedef struct
Disk
{
    zpl_vec3 center;
    zpl_vec3 normal;
    float radius;
}
Disk;

typedef struct
Obb
{
    zpl_mat4 mtx;
}
Obb;

typedef struct
Sphere
{
    zpl_vec3 center;
    float radius;
}
Sphere;


typedef struct
Primitive
{
    uint32_t StartIndex;
    uint32_t IndicesCount;
    uint32_t StartVertex;
    uint32_t VerticesCount;
    
    Sphere Sphere;
    Aabb Aabb;
    Obb Obb;
}
Primitive;

typedef Primitive* PrimitiveArray;

typedef struct
Group
{
    bgfx_vertex_buffer_handle_t VertexBuffer;
    bgfx_index_buffer_handle_t IndexBuffer;
    uint16_t VerticesCount;
    uint8_t* Vertices;
    uint32_t IndicesCount;
    uint16_t* Indices;
    Sphere Sphere;
    Aabb Aabb;
    Obb Obb;
    PrimitiveArray Primitives;
}
Group;

typedef Group* GroupArray;

typedef struct
Mesh
{
    bgfx_vertex_layout_t VertexLayout;
    GroupArray Groups;
}
Mesh;

typedef struct
Effect
{
    bgfx_program_handle_t ShaderProgram;
    bgfx_shader_handle_t VertexShader;
    bgfx_shader_handle_t PixelShader;
    bgfx_transform_t ViewTransform;
}
Effect;

typedef struct
Texture
{
    bgfx_texture_handle_t BGFXtexture;
    uint32_t Width;
    uint32_t Height;
    uint32_t BPP;
    Buffer PixelBuffer;
}
Texture;

typedef struct
VirtualTexture
{
    Texture* Texture;
    Rect Rect;
}
VirtualTexture;

ZPL_TABLE( , VTexHT, VTex, VirtualTexture)

typedef struct
RendererState
{
    DrawCommand* DrawCommands;
    int DrawCommandCount;
}
RendererState;

typedef struct
Animation
{
    int CurrentFrame;
    int MaxFrame;
    char* Name;
    float DuerationPerFrame;
}
Animation;

typedef struct
AppContext
{
    float dt;
    int width;
    int height;
    int TitleBarHeight;
    int MinMaxCloseButtonWidth;
    int MinMaxCloseButtonPadding;
    UINT NCMouseButton;
    LPARAM NCMousePos;
    void* Win32MessageProcFiber;
    void* Win32MainThreadFiber;
    uint8_t Win32VKeys[256];
    bool Running;
    bool redraw;
    bool resizebuffer;
    zpl_vec2 MousePos;
    uint32_t WindowColor;
}
AppContext;

typedef struct
AttribToId
{
    bgfx_attrib_t attr;
    uint16_t id;
}
AttribToId;

static AttribToId s_attribToId[] =
{
    // NOTICE:
    // Attrib must be in order how it appears in Attrib::Enum! id is
    // unique and should not be changed if new Attribs are added.
    { BGFX_ATTRIB_POSITION,  0x0001 },
    { BGFX_ATTRIB_NORMAL,    0x0002 },
    { BGFX_ATTRIB_TANGENT,   0x0003 },
    { BGFX_ATTRIB_BITANGENT, 0x0004 },
    { BGFX_ATTRIB_COLOR0,    0x0005 },
    { BGFX_ATTRIB_COLOR1,    0x0006 },
    { BGFX_ATTRIB_COLOR2,    0x0018 },
    { BGFX_ATTRIB_COLOR3,    0x0019 },
    { BGFX_ATTRIB_INDICES,   0x000e },
    { BGFX_ATTRIB_WEIGHT,    0x000f },
    { BGFX_ATTRIB_TEXCOORD0, 0x0010 },
    { BGFX_ATTRIB_TEXCOORD1, 0x0011 },
    { BGFX_ATTRIB_TEXCOORD2, 0x0012 },
    { BGFX_ATTRIB_TEXCOORD3, 0x0013 },
    { BGFX_ATTRIB_TEXCOORD4, 0x0014 },
    { BGFX_ATTRIB_TEXCOORD5, 0x0015 },
    { BGFX_ATTRIB_TEXCOORD6, 0x0016 },
    { BGFX_ATTRIB_TEXCOORD7, 0x0017 },
};

#define BX_COUNTOF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

bgfx_attrib_t idToAttrib(uint16_t id)
{
    for (uint32_t ii = 0; ii < BX_COUNTOF(s_attribToId); ++ii)
    {
        if(s_attribToId[ii].id == id)
        {
            return s_attribToId[ii].attr;
        }
    }
    return BGFX_ATTRIB_COUNT;
}

uint16_t attribToId(bgfx_attrib_t _attr)
{
    return s_attribToId[_attr].id;
}

typedef struct
AttribTypeToId
{
    bgfx_attrib_type_t type;
    uint16_t id;
}
AttribTypeToId;

AttribTypeToId s_attribTypeToId[] =
{
    // NOTICE:
    // AttribType must be in order how it appears in AttribType::Enum!
    // id is unique and should not be changed if new AttribTypes are
    // added.
    { BGFX_ATTRIB_TYPE_UINT8,  0x0001 },
    { BGFX_ATTRIB_TYPE_UINT10, 0x0005 },
    { BGFX_ATTRIB_TYPE_INT16,  0x0002 },
    { BGFX_ATTRIB_TYPE_HALF,   0x0003 },
    { BGFX_ATTRIB_TYPE_FLOAT,  0x0004 },
};

bgfx_attrib_type_t idToAttribType(uint16_t id)
{
    for (uint32_t ii = 0; ii < BX_COUNTOF(s_attribTypeToId); ++ii)
    {
        if (s_attribTypeToId[ii].id == id)
        {
            return s_attribTypeToId[ii].type;
        }
    }

    return BGFX_ATTRIB_TYPE_COUNT;
}

uint16_t attribTypeToId(bgfx_attrib_type_t _attr)
{
    return s_attribTypeToId[_attr].id;
}

#undef BX_COUNTOF

Mesh LoadMesh(zpl_allocator allocator,const char* meshfile)
{
    Mesh Result;
    zpl_array_init(Result.Groups,allocator);
#define BX_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32_t)(_a) | ( (uint32_t)(_b) << 8) | ( (uint32_t)(_c) << 16) | ( (uint32_t)(_d) << 24) ) )
#define BGFX_CHUNK_MAGIC_VB  BX_MAKEFOURCC('V', 'B', ' ', 0x1)
#define BGFX_CHUNK_MAGIC_VBC BX_MAKEFOURCC('V', 'B', 'C', 0x0)
#define BGFX_CHUNK_MAGIC_IB  BX_MAKEFOURCC('I', 'B', ' ', 0x0)
#define BGFX_CHUNK_MAGIC_IBC BX_MAKEFOURCC('I', 'B', 'C', 0x1)
#define BGFX_CHUNK_MAGIC_PRI BX_MAKEFOURCC('P', 'R', 'I', 0x0)

    zpl_file MeshFile;
    zpl_file_open(&MeshFile,meshfile);
    zpl_file_dirinfo_refresh(&MeshFile);
    zpl_i64 filesize =  zpl_file_size(&MeshFile);
    uint32_t chunk;
    zpl_isize bytesread = 0;
    while(1 == zpl_file_read(&MeshFile,(void *)&chunk,sizeof(uint32_t))
          && filesize > zpl_file_tell(&MeshFile))
    {
        Group CurrentMeshDataGroup;
        switch(chunk)
        {
            case BGFX_CHUNK_MAGIC_VB:
            {
                zpl_file_read(&MeshFile,(void *)&(CurrentMeshDataGroup.Sphere),sizeof(Sphere));
                zpl_file_read(&MeshFile,(void *)&(CurrentMeshDataGroup.Aabb),sizeof(Aabb));
                zpl_file_read(&MeshFile,(void *)&(CurrentMeshDataGroup.Obb),sizeof(Obb));
                uint8_t numAttrs;
                zpl_file_read(&MeshFile,(void *)&numAttrs, sizeof(uint8_t));

                zpl_file_read(&MeshFile,(void *)(&Result.VertexLayout.stride),sizeof(uint16_t));

                bgfx_vertex_layout_begin(&Result.VertexLayout,BGFX_RENDERER_TYPE_NOOP);

                for (uint32_t ii = 0; ii < numAttrs; ++ii)
                {
                    uint16_t offset;
                    zpl_file_read(&MeshFile, &offset,sizeof(uint16_t));

                    uint16_t attribId = 0;
                    zpl_file_read(&MeshFile, &attribId, sizeof(uint16_t));

                    uint8_t num;
                    zpl_file_read(&MeshFile, &num, sizeof(uint8_t));

                    uint16_t attribTypeId;
                    zpl_file_read(&MeshFile, &attribTypeId, sizeof(uint16_t));

                    bool normalized;
                    zpl_file_read(&MeshFile, &normalized, sizeof(bool));

                    bool asInt;
                    zpl_file_read(&MeshFile, &asInt, sizeof(bool));
            
                    bgfx_attrib_t     attr = idToAttrib(attribId);
                    bgfx_attrib_type_t type = idToAttribType(attribTypeId);
                    if (BGFX_ATTRIB_COUNT     != attr
                        &&  BGFX_ATTRIB_TYPE_COUNT != type)
                    {
                        bgfx_vertex_layout_add(&(Result.VertexLayout),attr, num, type, normalized, asInt);
                        Result.VertexLayout.offset[attr] = offset;
                    }
                }

                bgfx_vertex_layout_end(&Result.VertexLayout);
                uint16_t Stride = Result.VertexLayout.stride;
                zpl_file_read(&MeshFile,(void *)&(CurrentMeshDataGroup.VerticesCount),sizeof(uint16_t));
                const bgfx_memory_t* Mem = bgfx_alloc(Stride*CurrentMeshDataGroup.VerticesCount);
                zpl_file_read(&MeshFile,(void *)Mem->data,Mem->size);
                CurrentMeshDataGroup.Vertices = (uint8_t *)Mem->data;
                CurrentMeshDataGroup.VertexBuffer = bgfx_create_vertex_buffer(Mem,&Result.VertexLayout,BGFX_BUFFER_NONE);
            }
            break;
            case BGFX_CHUNK_MAGIC_VBC:
            {

            }
            break;
            case BGFX_CHUNK_MAGIC_IB:
            {
                zpl_file_read(&MeshFile,(void *)&CurrentMeshDataGroup.IndicesCount,sizeof(uint32_t));
                const bgfx_memory_t* Mem = bgfx_alloc(CurrentMeshDataGroup.IndicesCount*2);
                zpl_file_read(&MeshFile,(void *)Mem->data,Mem->size);
                CurrentMeshDataGroup.IndexBuffer = bgfx_create_index_buffer(Mem,BGFX_BUFFER_NONE);                      
            }
            break;
            case BGFX_CHUNK_MAGIC_IBC:
            {

            }
            break;
            case BGFX_CHUNK_MAGIC_PRI:
            {
                uint16_t len = {};
                zpl_file_read(&MeshFile,(void *)&len,sizeof(uint16_t));
                char *string = (char *)alloca(len+1);
                string[len] = 0;
                zpl_file_read(&MeshFile,(void *)string,len);
                uint16_t num = {};
                zpl_file_read(&MeshFile,(void *)&num,sizeof(uint16_t));
                CurrentMeshDataGroup.Primitives = zpl_alloc_array(allocator,Primitive,num);
                for(uint16_t x = 0; x < num ;x++)
                {
                    zpl_file_read(&MeshFile,(void *)&len,sizeof(uint16_t));
                    char *name = (char *)alloca(len + 1);
                    name[len] = 0;
                    zpl_file_read(&MeshFile,(void *)name,len);
                    Primitive Prim = {};
                    zpl_file_read(&MeshFile,(void *)&Prim.StartIndex,sizeof(uint32_t));
                    zpl_file_read(&MeshFile,(void *)&Prim.IndicesCount,sizeof(uint32_t));
                    zpl_file_read(&MeshFile,(void *)&Prim.StartVertex,sizeof(uint32_t));
                    zpl_file_read(&MeshFile,(void *)&Prim.VerticesCount,sizeof(uint32_t));
                    zpl_file_read(&MeshFile,(void *)&Prim.Sphere,sizeof(Sphere));
                    zpl_file_read(&MeshFile,(void *)&Prim.Aabb,sizeof(Aabb));
                    zpl_file_read(&MeshFile,(void *)&Prim.Obb,sizeof(Obb));
                    CurrentMeshDataGroup.Primitives[x] = Prim;
                }
                zpl_array_append(Result.Groups,CurrentMeshDataGroup);
                CurrentMeshDataGroup = {};
            }
            break;

        }
    }
    zpl_file_close(&MeshFile);
#undef BGFX_CHUNK_MAGIC_VB
#undef BGFX_CHUNK_MAGIC_VBC
#undef BGFX_CHUNK_MAGIC_IB
#undef BGFX_CHUNK_MAGIC_IBB
#undef BGFX_CHUNK_MAGIC_PRI
#undef BX_MAKEFOURCC
    return Result;
}

void RenderMesh(Mesh* mesh,bgfx_view_id_t id,Effect *effect,float* transform,uint64_t state = BGFX_STATE_MASK)
{
    if (BGFX_STATE_MASK == state)
    {
        state = 0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA
        ;
    }
    bgfx_set_transform(transform,1);
    bgfx_set_state(state,0);
    
    for(int i = 0;i < zpl_array_count(mesh->Groups); i++)
    {
        Group* Group = mesh->Groups + i;
        
        bgfx_set_index_buffer(Group->IndexBuffer,0,UINT32_MAX);
        bgfx_set_vertex_buffer(0, Group->VertexBuffer,0,UINT32_MAX);
        bgfx_submit(id, effect->ShaderProgram, 0, i != zpl_array_count(mesh->Groups)-1);
    }
}

Effect LoadShader(zpl_allocator allocator,const char* vsfile,const char* psfile)
{
    Effect Result = {};
    zpl_file_contents vsdata = zpl_file_read_contents(allocator,false,vsfile);
    zpl_file_contents psdata = zpl_file_read_contents(allocator,false,psfile);
    Result.VertexShader = bgfx_create_shader(bgfx_make_ref(vsdata.data,vsdata.size));
    Result.PixelShader = bgfx_create_shader(bgfx_make_ref(psdata.data,psdata.size));
    Result.ShaderProgram = bgfx_create_program(Result.VertexShader,Result.PixelShader,true);
    return Result;
}

Texture LoadPNG(zpl_allocator allocator,const char* file)
{
    Texture Result = {};
    zpl_file_contents File =  zpl_file_read_contents(allocator,false,file);
    Result.BPP = 4;
    lodepng_decode_memory((unsigned char **)(&Result.PixelBuffer.data),&Result.Width,&Result.Height,(const unsigned char*)File.data,File.size,LCT_RGBA,32 / Result.BPP);
    Result.PixelBuffer.size = Result.Width*Result.Height*Result.BPP;
    uint32_t* pixelbase = (uint32_t *)(Result.PixelBuffer.data );
    Result.BGFXtexture = bgfx_create_texture_2d(Result.Width,Result.Height,false,1,BGFX_TEXTURE_FORMAT_RGBA8,BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT,bgfx_make_ref(Result.PixelBuffer.data,Result.PixelBuffer.size));
    return Result;
}

zpl_json_object ParseJSON(zpl_allocator allocator,const char* file,bool hascomments)
{
    zpl_json_object Result ={};
    zpl_u8 Error;
    zpl_file_contents File =  zpl_file_read_contents(allocator,false,file);
    zpl_json_parse(&Result,File.size,(char *const)(File.data),zpl_heap_allocator(),hascomments,&Error);
    return Result;
}

zpl_json_object* SearchJSON(const char* Param , zpl_json_object *obj)
{
    return zpl_json_find(obj,Param,false);
}

void Win32EnableHIDPISupport()
{
    if(!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
    {
        if(!SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE))
        {
            SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
        }
    }
}


LRESULT CALLBACK Win32WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    AppContext* App = (AppContext *)GetWindowLongPtr(hwnd,GWLP_USERDATA); 
    LRESULT result = {};
    if(ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
    {
        return true;
    }
    switch(uMsg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;
        case WM_NCCALCSIZE:
        {

        }
        break;
        case WM_SIZE:
        {
            if(App)
            {
                App->resizebuffer = true;
                int newwidth = LOWORD(lParam) ;
                int newheight =HIWORD(lParam) ;
                    
                {
                    if(newwidth != App->width)
                    {
                        App->width = newwidth;
                    }
                    if(newheight != App->height)
                    {
                        App->height = newheight;
                    }
                }
            }
            
        }
        break;
                
        case WM_CREATE:
        {

        }
        break;
        case WM_WINDOWPOSCHANGING:
        {
            if(App)
            {
                WINDOWPOS WPos = *((WINDOWPOS *) lParam);
                App->resizebuffer = true;
                
            }
        }
        break;
        case WM_TIMER:
        {
            if(wParam==1)
            {
                SwitchToFiber(App->Win32MainThreadFiber);
            }
        }
        break;
        case WM_ENTERSIZEMOVE:
        case WM_ENTERMENULOOP:
        {
            SetTimer(hwnd,1,1,NULL);
        }
        break;
        case WM_EXITSIZEMOVE:
        case WM_EXITMENULOOP:
        {
            KillTimer(hwnd, 1);
        }
        break;
        case WM_NCMOUSEMOVE:
        {
            if(App)
            {
                if(App->NCMouseButton == WM_NCLBUTTONDOWN)
                {
                    if(GET_X_LPARAM(App->NCMousePos) != GET_X_LPARAM(lParam) ||
                       GET_Y_LPARAM(App->NCMousePos) != GET_Y_LPARAM(lParam))
                    {
                        DefWindowProc(hwnd,WM_NCLBUTTONDOWN,HTCAPTION,App->NCMousePos);
                        App->NCMouseButton = 0;
                    }

                }
            }
        }
        break;
        case WM_MOUSEMOVE:
        {
            if(App)
            {
                if(App->NCMouseButton == WM_NCLBUTTONDOWN)
                {
                    if(GET_X_LPARAM(App->NCMousePos) != GET_X_LPARAM(lParam) ||
                       GET_Y_LPARAM(App->NCMousePos) != GET_Y_LPARAM(lParam))
                    {
                        DefWindowProc(hwnd,WM_NCLBUTTONDOWN,HTCAPTION,App->NCMousePos);
                        App->NCMouseButton = 0;
                    }

                }

                App->MousePos.x = GET_X_LPARAM(lParam);
                App->MousePos.y = GET_Y_LPARAM(lParam);
            }
        }
        break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            if(wParam < 256)
            {
                uint8_t index = (wParam & 0xff);
                App->Win32VKeys[index] = 1;
            }
        }
        break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            if(wParam < 256)
            {
                uint8_t index = (wParam & 0xff);
                App->Win32VKeys[index] = 0;
            }            
        }
        break;
#if 0
        case WM_NCLBUTTONDOWN:
        {
            if(wParam == HTCLOSE || wParam == HTMINBUTTON || wParam == HTMAXBUTTON)
            {
                if(wParam == HTCLOSE)
                {
                    SendMessage(hwnd,WM_CLOSE,NULL,NULL); 
                }
                if(wParam == HTMINBUTTON)
                {
                    ShowWindow(hwnd,SW_MINIMIZE); 
                }
                if(wParam == HTMAXBUTTON)
                {
                    WINDOWPLACEMENT WndPlcmnt={};
                    WndPlcmnt.length = sizeof(WINDOWPLACEMENT);
                    GetWindowPlacement(hwnd,&WndPlcmnt);
                    if(WndPlcmnt.showCmd == SW_MAXIMIZE)
                    {
                        ShowWindow(hwnd,SW_RESTORE);                    
                    }
                    else
                    {
                        ShowWindow(hwnd,SW_MAXIMIZE);
                    }
                }
                return 0;
            }
        
            if(wParam == HTCAPTION && App)
            {
                App->NCMouseButton = uMsg;
                App->NCMousePos = lParam; 
                return 0;
            }

        }
        break;
        case WM_NCLBUTTONDBLCLK:
        {
            if(wParam == HTCLOSE || wParam == HTMINBUTTON || wParam == HTMAXBUTTON)
            {
                if(wParam == HTCLOSE)
                {
                    SendMessage(hwnd,WM_CLOSE,NULL,NULL); 
                }
                if(wParam == HTMINBUTTON)
                {
                    ShowWindow(hwnd,SW_MINIMIZE); 
                }
                if(wParam == HTMAXBUTTON)
                {
                    WINDOWPLACEMENT WndPlcmnt={};
                    WndPlcmnt.length = sizeof(WINDOWPLACEMENT);
                    GetWindowPlacement(hwnd,&WndPlcmnt);
                    if(WndPlcmnt.showCmd == SW_MAXIMIZE)
                    {
                        ShowWindow(hwnd,SW_RESTORE);                    
                    }
                    else
                    {
                        ShowWindow(hwnd,SW_MAXIMIZE);
                    }
                }
                return 0;
            }
        }
        break;
        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO mmi = (LPMINMAXINFO ) lParam;
            HMONITOR monitor = MonitorFromWindow(hwnd,MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX monitorinfo = {};
            monitorinfo.cbSize = sizeof(MONITORINFOEX);
            GetMonitorInfo(monitor,&monitorinfo);
            RECT rcWorkArea = monitorinfo.rcWork;
            RECT rcMonitorArea = monitorinfo.rcMonitor;
            mmi->ptMaxPosition.x = rcWorkArea.left - rcMonitorArea.left;
            mmi->ptMaxPosition.y = rcWorkArea.top - rcMonitorArea.top;
            mmi->ptMaxSize.x = rcWorkArea.right - rcMonitorArea.left;
            mmi->ptMaxSize.y = rcWorkArea.bottom - rcMonitorArea.top;
            mmi->ptMaxPosition.x = mmi->ptMaxPosition.x < 0 ? -mmi->ptMaxPosition.x : mmi->ptMaxPosition.x ;
            mmi->ptMaxPosition.y = mmi->ptMaxPosition.y < 0 ? -mmi->ptMaxPosition.y : mmi->ptMaxPosition.y ;
            mmi->ptMaxSize.x = mmi->ptMaxSize.x < 0 ? -mmi->ptMaxSize.x : mmi->ptMaxSize.x ;
            mmi->ptMaxSize.y = mmi->ptMaxSize.y < 0 ? -mmi->ptMaxSize.y : mmi->ptMaxSize.y ;
            mmi->ptMaxSize.x -= 1;
            //mmi->ptMaxSize.y += 8;
            return 0;
        }
        break;
        case WM_NCHITTEST:
        {
            if(App)
            {
                // Acquire the window rect
                RECT WindowRect={};
                GetWindowRect(hwnd, &WindowRect);
                int offset = App->MinMaxCloseButtonPadding;
                int caption_height = App->TitleBarHeight;
                int buttonwidth = App->MinMaxCloseButtonWidth;
                POINT CursorPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                result = HTNOWHERE;
                if (CursorPos.x > WindowRect.left && CursorPos.x < WindowRect.right)
                {
                    if (CursorPos.y < WindowRect.top + caption_height)
                    {
                        result = HTCAPTION;
                        if(CursorPos.x > (WindowRect.right - buttonwidth*3)  ) result = HTMINBUTTON;  
                        if(CursorPos.x > (WindowRect.right - buttonwidth*2)  ) result = HTMAXBUTTON;  
                        if(CursorPos.x > (WindowRect.right - buttonwidth*1)  ) result = HTCLOSE;  
                    }
                }
            
                if (CursorPos.x > WindowRect.left && CursorPos.x < WindowRect.right)
                {
                    if (CursorPos.y < WindowRect.top + offset)
                    {
                        result = HTTOP;
                    }
                    else if (CursorPos.y > WindowRect.bottom - offset)
                    {
                        result = HTBOTTOM;
                    }
                }
                if (CursorPos.y > WindowRect.top && CursorPos.y < WindowRect.bottom)
                {
                    if (CursorPos.x < WindowRect.left + offset)
                    {
                        result = HTLEFT;
                    }
                    else if (CursorPos.x > WindowRect.right - offset)
                    {
                        result = HTRIGHT;
                    }
                }

                if (CursorPos.y < WindowRect.top + offset && CursorPos.x < WindowRect.left + offset)
                {
                    result = HTTOPLEFT;
                }
                if (CursorPos.y < WindowRect.top + offset && CursorPos.x > WindowRect.right - offset) result = HTTOPRIGHT;
                if (CursorPos.y > WindowRect.bottom - offset && CursorPos.x > WindowRect.right - offset) result = HTBOTTOMRIGHT;
                if (CursorPos.y > WindowRect.bottom - offset && CursorPos.x < WindowRect.left + offset) result = HTBOTTOMLEFT;

                if (result != HTNOWHERE) return result;
            }
        }
        break;
#endif
        default:
        {
        }
        break;
        
    }
    result = DefWindowProc( hwnd, uMsg, wParam, lParam);
    return result;
}

void CALLBACK Win32MessageProc(LPVOID lpFiberParameter)
{
    AppContext* App = (AppContext *)lpFiberParameter;
    MSG WindowMessage = {};
    for(;;)
    {
        while(PeekMessage(&WindowMessage,NULL,0,0,PM_REMOVE))
        {                    
            switch(WindowMessage.message)
            {
                case WM_QUIT:
                {
                    App->Running = false;
                }
                break;
                default:
                {
                    TranslateMessage(&WindowMessage);
                    DispatchMessageW(&WindowMessage);
                }
                break;
            }
        }
        SwitchToFiber(App->Win32MainThreadFiber);
    }

}

void CALLBACK Win32FallbackMessageProc(LPVOID lpFiberParameter)
{
    AppContext* App = (AppContext *)lpFiberParameter;
    MSG WindowMessage = {};
    {
        while(PeekMessage(&WindowMessage,NULL,0,0,PM_REMOVE))
        {
            switch(WindowMessage.message)
            {
                case WM_QUIT:
                {
                    App->Running = false;
                }
                break;
                default:
                {
                }
                break;
            }
            TranslateMessage(&WindowMessage);
            DispatchMessage(&WindowMessage);
        }
    }

}



typedef struct
VertexPositionColor
{
    zpl_vec3 Position;
    uint32_t Color;
}
VertexPositionColor;

typedef struct
VertexPositionTexture
{
    zpl_vec3 Position;
    zpl_vec2 TexUV;
}
VertexPositionTexture;

typedef struct
VertexPositionTextureColor
{
    zpl_vec3 Position;
    zpl_vec2 TexUV;
    uint32_t Color;
}
VertexPositionTextureColor;

void setupStyle(bool _dark)
{
    // Doug Binks' darl color scheme
    // https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9
    ImGuiStyle& style = ImGui::GetStyle();
    if (_dark)
    {
        ImGui::StyleColorsDark(&style);
    }
    else
    {
        ImGui::StyleColorsLight(&style);
    }

    style.FrameRounding    = 4.0f;
    style.WindowBorderSize = 0.0f;
}


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,int nShowCmd)
{
    zpl_virtual_memory MemoryBuffer = zpl_vm_alloc(NULL,zpl_megabytes(256));
    zpl_arena ArenaMem = {};
    zpl_arena_init_from_memory(&ArenaMem,MemoryBuffer.data,MemoryBuffer.size);
    zpl_allocator ArenaAllocator = zpl_arena_allocator(&ArenaMem);
    AppContext App = {};
    App.width = 800;
    App.height = 600;
    App.TitleBarHeight = 30;
    App.MinMaxCloseButtonWidth = 60;
    App.MinMaxCloseButtonPadding = 2;
    App.WindowColor = 0xff1c1c1c;
    App.Win32MainThreadFiber = ConvertThreadToFiber(NULL);
    App.Win32MessageProcFiber = CreateFiber(0,&Win32MessageProc,(void *)&App);
    //Win32EnableHIDPISupport();
    WNDCLASSEX WNDCLS = {};
    WNDCLS.cbSize = sizeof(WNDCLASSEX);
    WNDCLS.style = CS_HREDRAW | CS_VREDRAW|CS_OWNDC;
    WNDCLS.hInstance = hInstance;
    WNDCLS.hCursor = LoadCursor(NULL,IDC_ARROW);
    WNDCLS.lpszClassName = "testwindowclass";
    WNDCLS.lpfnWndProc = Win32WindowProcedure;
    WNDCLS.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH); 
    RegisterClassEx(&WNDCLS);
    HWND hwnd = CreateWindowEx(NULL, WNDCLS.lpszClassName,"NPR Engine",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,App.width,App.height,NULL,NULL,WNDCLS.hInstance,NULL);
    //SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE)&~WS_SIZEBOX);
    SetWindowLongPtr(hwnd, GWLP_USERDATA ,(LONG_PTR) &App);
    ShowWindow(hwnd,SW_HIDE);
    {
        bgfx_platform_data_t* BGFXPlatformData = (bgfx_platform_data_t *)zpl_alloc(ArenaAllocator, sizeof(bgfx_platform_data_t));
        BGFXPlatformData->nwh = (void *)hwnd;
        bgfx_set_platform_data(BGFXPlatformData);
    }
    IMGUI_CHECKVERSION();
    bgfx_init_t BGFXInit = {};
    bgfx_init_ctor(&BGFXInit);
    BGFXInit.type = BGFX_RENDERER_TYPE_DIRECT3D11;
    BGFXInit.vendorId = BGFX_PCI_ID_NONE;
    BGFXInit.resolution.width = App.width;
    BGFXInit.resolution.height = App.height;
    BGFXInit.resolution.reset = BGFX_RESET_VSYNC;
    bool set = bgfx_init(&BGFXInit);
    //bgfx_set_debug(BGFX_DEBUG_STATS);
    bgfx_set_view_mode(255,BGFX_VIEW_MODE_SEQUENTIAL);
    bgfx_set_view_mode(0,BGFX_VIEW_MODE_SEQUENTIAL);
    ShowWindow(hwnd,SW_SHOW);
    App.Running = true;
    App.dt = 0;
    LARGE_INTEGER PerfCounter;
    QueryPerformanceCounter(&PerfCounter);
    App.resizebuffer = true;
    App.redraw = false;
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    {
        ImGuiIO* io = &ImGui::GetIO();
        io->Fonts->AddFontDefault();
        unsigned char* pixels;
        int fwidth, fheight, fbpp;
        io->Fonts->GetTexDataAsRGBA32(&pixels, &fwidth, &fheight, &fbpp);
        io->Fonts->TexID = (ImTextureID)bgfx_create_texture_2d(fwidth,fheight,false,1,BGFX_TEXTURE_FORMAT_RGBA8,BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT,bgfx_make_ref(pixels,fwidth*fheight*fbpp)).idx;    
        setupStyle(true);
    }
    Mesh CubeMesh = LoadMesh(ArenaAllocator,"bunny.bin");
    bgfx_uniform_handle_t u_time = bgfx_create_uniform("u_time", BGFX_UNIFORM_TYPE_VEC4,1);
    Effect ImGuiShader = LoadShader(ArenaAllocator,"vs_imgui.bin","fs_imgui.bin");
    Effect MeshShader = LoadShader(ArenaAllocator,"vs_mesh.bin","fs_mesh.bin");
    uint32_t WhiteTex[] = {0xffffffff};
    bgfx_texture_handle_t WhitePixelTexture = bgfx_create_texture_2d(1,1,false,1,BGFX_TEXTURE_FORMAT_RG8,BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT,bgfx_make_ref(WhiteTex,sizeof(WhiteTex)));
    bgfx_uniform_handle_t TextureUniform = bgfx_create_uniform("s_texture",BGFX_UNIFORM_TYPE_SAMPLER,1);
    bgfx_vertex_layout_t ImGuiVertexLayout = {};
    bgfx_vertex_layout_begin(&ImGuiVertexLayout,BGFX_RENDERER_TYPE_NOOP);
    bgfx_vertex_layout_add(&ImGuiVertexLayout,BGFX_ATTRIB_POSITION,2,BGFX_ATTRIB_TYPE_FLOAT,false,false);
    bgfx_vertex_layout_add(&ImGuiVertexLayout,BGFX_ATTRIB_TEXCOORD0,2,BGFX_ATTRIB_TYPE_FLOAT,false,false);
    bgfx_vertex_layout_add(&ImGuiVertexLayout,BGFX_ATTRIB_COLOR0,4,BGFX_ATTRIB_TYPE_UINT8,true,false);
    bgfx_vertex_layout_end(&ImGuiVertexLayout);
    bgfx_set_view_clear(0,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH,0x303030ff,1,0);
    zpl_vec3 at  = { 0.0f, 1.0f,  0.0f };
    zpl_vec3 eye = { 0.0f, 1.0f,  2.5f };
    zpl_f64 prevtime = zpl_time_now();
    while(App.Running)
    {
        SwitchToFiber(App.Win32MessageProcFiber);
        if(!App.Running) break;
        LARGE_INTEGER NewPerfCounter;
        QueryPerformanceCounter(&NewPerfCounter);
        LARGE_INTEGER PerfFrequency;
        QueryPerformanceFrequency(&PerfFrequency);
        App.dt = ((float)(NewPerfCounter.QuadPart - PerfCounter.QuadPart) )*1000000;
        App.dt /= ((float)PerfFrequency.QuadPart);
        App.dt /= 1000000;
        PerfCounter = NewPerfCounter;
        {
            ImGuiIO* io = &ImGui::GetIO();
            io->DisplaySize = ImVec2(App.width, App.height);
            io->DeltaTime   = App.dt;
            io->IniFilename = NULL;
        }
        if(App.resizebuffer)
        {
            bgfx_reset(App.width,App.height,BGFX_RESET_NONE,BGFXInit.resolution.format);
            App.resizebuffer = false;
        }
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        if(ImGui::Begin("NPR Engine DEBUG"))
        {
            ImGui::Text("NPR Debug Test");
            ImGui::SliderFloat3("At:",at.e,-10,10);
            ImGui::SliderFloat3("Eye:",eye.e,-10,10);
                
        }
        ImGui::End();
        ImGui::EndFrame();
        ImGui::Render();
        bgfx_set_view_rect(255,0,0,App.width,App.height);
        bgfx_set_view_rect(0,0,0,App.width,App.height);
        bgfx_touch(0);
        float time = (float)(zpl_time_now() - prevtime);
        time = 0;
        bgfx_set_uniform(u_time,&time,1);
        const bgfx_caps_t* caps = bgfx_get_caps();
        {
            zpl_vec3 up = { 0.0f, 1.0f, 0.0f };
            zpl_mat4 view = {};
            zpl_mat4_look_at(&view, eye, at,up);
            zpl_mat4 proj = {};
            zpl_mat4_perspective_DX(&proj, zpl_to_radians(60.0f), float(App.width)/float(App.height), 0.1f, 100.0f);
            bgfx_set_view_transform(0, view.e, proj.e);
        }
        zpl_mat4 mtx;
        zpl_mat4_rotate(&mtx,{0,1,0},time*0.37f);
        RenderMesh(&CubeMesh,0,&MeshShader,mtx.e);
        
        // Draw ImGUI
        if(true)
        {
            ImDrawData* _drawData = ImGui::GetDrawData();
            int viewID = 255; 
            {
                zpl_mat4 ortho = {};
                zpl_mat4 identity = {};
                zpl_mat4_identity(&identity);
                zpl_mat4_ortho3d(&ortho, 0.0f, App.width, App.height, 0.0f, 1000.0f, 0.0f);
                bgfx_set_view_transform(viewID, identity.e, ortho.e);
            }
            // Render command lists
            for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
            {
                bgfx_transient_vertex_buffer_t tvb;
                bgfx_transient_index_buffer_t tib;

                const ImDrawList* drawList = _drawData->CmdLists[ii];
                uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
                uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();

                bgfx_alloc_transient_vertex_buffer(&tvb, numVertices, &ImGuiVertexLayout);
                bgfx_alloc_transient_index_buffer(&tib, numIndices);

                ImDrawVert* verts = (ImDrawVert*)tvb.data;
                zpl_memcopy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert) );

                ImDrawIdx* indices = (ImDrawIdx*)tib.data;
                zpl_memcopy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

                uint32_t offset = 0;
                for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
                {
                    if (cmd->UserCallback)
                    {
                        cmd->UserCallback(drawList, cmd);
                    }
                    else if (cmd->ElemCount > 0)
                    {
                        uint64_t state = 0
                            | BGFX_STATE_WRITE_RGB
                            | BGFX_STATE_WRITE_A
                            | BGFX_STATE_MSAA
                            ;

                        bgfx_texture_handle_t TexHandle  = {};
                      
                        if (cmd->TextureId != NULL)
                        {
                            TexHandle.idx = (uint16_t)cmd->TextureId;
                        }
                        else
                        {
                            TexHandle.idx = WhitePixelTexture.idx;
                        }
                        state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);

                        const uint16_t xx = uint16_t((cmd->ClipRect.x > 0.0f ? cmd->ClipRect.x : 0.0f) );
                        const uint16_t yy = uint16_t((cmd->ClipRect.y > 0.0f ? cmd->ClipRect.y : 0.0f) );
                        bgfx_set_scissor(xx, yy
                                         , uint16_t(( cmd->ClipRect.z > 65535.0f ? 65535.0f : cmd->ClipRect.z )-xx)
                                         , uint16_t(( cmd->ClipRect.w > 65535.0f ? 65535.0f : cmd->ClipRect.w)-yy)
                                         );

                        bgfx_set_state(state,0);
                        bgfx_set_texture(0, TextureUniform, TexHandle,UINT32_MAX);
                        bgfx_set_transient_vertex_buffer(0, &tvb, 0, numVertices);
                        bgfx_set_transient_index_buffer(&tib, offset, cmd->ElemCount);
                        bgfx_submit(viewID, ImGuiShader.ShaderProgram,0,false);
                    }

                    offset += cmd->ElemCount;       
                }
            }
            
        }
         bgfx_frame(false);
    }
    ImGui_ImplWin32_Shutdown();
    return 0;
}
