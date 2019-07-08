/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"

namespace
{

//顶点以及纹理坐标对象
struct PosTexVertex
{
    float m_x;
    float m_y;
    float m_z;
    float t_x;
    float t_y;
    
    static void init()
    {
        ms_decl
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0,   2, bgfx::AttribType::Float)
        .end();
    };
    
    static bgfx::VertexDecl ms_decl;
};
    
bgfx::VertexDecl PosTexVertex::ms_decl;
//顶点坐标数据
static PosTexVertex s_vertices[] =
{
    {-1.0f,  1.0f,  0.0f, 0.0f, 0.0f },
    { 1.0f,  1.0f,  0.0f, 1.0f, 0.0f },
    {-1.0f, -1.0f,  0.0f, 0.0f, 1.0f },
    { 1.0f, -1.0f,  0.0f, 1.0f, 1.0f },
};
//顶点顺序
static const uint16_t s_triList[] =
{
    0, 1, 2, // 0
    1, 3, 2,
};
    
class ExampleFilter : public entry::AppI
{
public:
	ExampleFilter(const char* _name, const char* _description)
		: entry::AppI(_name, _description)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_TEXT;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type     = args.m_type;
		init.vendorId = args.m_pciId;
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		init.resolution.reset  = m_reset;
		bgfx::init(init);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
			);
        bgfx::setViewClear(1
                           , BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
                           , 0x303030ff
                           , 1.0f
                           , 0
                           );

        // Create vertex stream declaration.
        PosTexVertex::init();
        
        // Create static vertex buffer.
        m_vbh = bgfx::createVertexBuffer(
                                         // Static data can be passed with bgfx::makeRef
                                         bgfx::makeRef(s_vertices, sizeof(s_vertices) )
                                         , PosTexVertex::ms_decl
                                         );
        
        // Create static index buffer for triangle list rendering.
        m_ibh = bgfx::createIndexBuffer(
                                           // Static data can be passed with bgfx::makeRef
                                           bgfx::makeRef(s_triList, sizeof(s_triList) )
                                           );
        
        // Create program from shaders.
        m_program = loadProgram("vs_filter_normal", "fs_filter_normal");
        m_chain_program = loadProgram("vs_filter_normal", "fs_filter_bw");
        m_texture = loadTexture("images/fengjing.jpg");
        u_Texture = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        
        m_frameBuffer = bgfx::createFrameBuffer(m_width, m_height, bgfx::TextureFormat::BGRA8);
        
		imguiCreate();
	}

	virtual int shutdown() override
	{
		imguiDestroy();
        
        bgfx::destroy(m_ibh);
        bgfx::destroy(m_vbh);
        bgfx::destroy(m_program);
        bgfx::destroy(m_texture);
        bgfx::destroy(u_Texture);
        bgfx::destroy(m_chain_program);
        bgfx::destroy(m_frameBuffer);

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
			imguiBeginFrame(m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			showExampleDialog(this);

			imguiEndFrame();

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);
            
            uint64_t state = 0
            | BGFX_STATE_WRITE_R
            | BGFX_STATE_WRITE_G
            | BGFX_STATE_WRITE_B
            | BGFX_STATE_WRITE_A
            | UINT64_C(0)
            ;
            
            // This dummy draw call is here to make sure that view 0 is cleared
            // if no other draw calls are submitted to view 0.
            bgfx::touch(0);
            bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );//设置View0的视口
            bgfx::setViewFrameBuffer(0, m_frameBuffer);//设置绘制到FrameBuffer上
            
            bgfx::touch(1);
            bgfx::setViewRect(1, 0, 0, uint16_t(m_width), uint16_t(m_height) );
            bgfx::setViewFrameBuffer(1, BGFX_INVALID_HANDLE);
            
            //声明和设置我们的绘制顺序
            bgfx::ViewId order[] =
            {
                0,1
            };
            bgfx::setViewOrder(0, BX_COUNTOF(order), order);
            
            bgfx::setVertexBuffer(0, m_vbh);//设置stream0的vertexBuffer，注意第一个参数不是viewId
            bgfx::setIndexBuffer(m_ibh);//设置顶点索引buffer数据
            bgfx::setState(state);//设置控制绘制信息的标志位
            bgfx::setTexture(0, u_Texture, m_texture);//设置对应的u_Texture这个着色器参数的纹理资源
            bgfx::submit(0, m_program);//提交绘制单张纹理的Program
            
            bgfx::setVertexBuffer(0, m_vbh);
            bgfx::setIndexBuffer(m_ibh);
            bgfx::setState(state);
            bgfx::setTexture(0, u_Texture, bgfx::getTexture(m_frameBuffer));//设置对应的u_Texture这个着色器参数的纹理资源，即上一次的绘制结果
            bgfx::submit(1, m_chain_program);

			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	entry::MouseState m_mouseState;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
    
    bgfx::VertexBufferHandle m_vbh;
    bgfx::IndexBufferHandle m_ibh;
    bgfx::TextureHandle m_texture;
    bgfx::UniformHandle u_Texture;
    bgfx::ProgramHandle m_program;
    bgfx::ProgramHandle m_chain_program;
    bgfx::FrameBufferHandle m_frameBuffer;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(ExampleFilter, "41-filter", "Initialization and show filter effects.");
