#include "stdafx.h"
#pragma hdrstop

#include "blender_light_point.h"

CBlender_accum_point::CBlender_accum_point() { description.CLS = 0; }
CBlender_accum_point::~CBlender_accum_point() {}
void CBlender_accum_point::Compile(CBlender_Compile& C)
{
    IBlender::Compile(C);

#if RENDER == R_R2
    BOOL b_HW_smap = RImplementation.o.HW_smap;
    BOOL b_HW_PCF = RImplementation.o.HW_smap_PCF;
    BOOL blend = RImplementation.o.fp16_blend;
    D3DBLEND dest = blend ? D3DBLEND_ONE : D3DBLEND_ZERO;
    switch (C.iElement)
    {
    case SE_L_FILL: // fill projective
        C.r_Pass("null", "copy", false, FALSE, FALSE);
        C.r_Sampler("s_base", C.L_textures[0]);
        C.r_End();
        break;
    case SE_L_UNSHADOWED: // unshadowed
        C.r_Pass("accum_volume", "accum_omni_unshadowed", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler_clf("s_lmap", *C.L_textures[0]);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        C.r_End();
        break;
    case SE_L_NORMAL: // normal
        C.r_Pass("accum_volume", "accum_omni_normal", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler("s_lmap", C.L_textures[0]);
        if (b_HW_smap)
        {
            if (b_HW_PCF)
                C.r_Sampler_clf("s_smap", r2_RT_smap_depth);
            else
                C.r_Sampler_rtf("s_smap", r2_RT_smap_depth);
        }
        else
            C.r_Sampler_rtf("s_smap", r2_RT_smap_surf);
        jitter(C);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        C.r_End();
        break;
    case SE_L_FULLSIZE: // normal-fullsize
        C.r_Pass("accum_volume", "accum_omni_normal", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler("s_lmap", C.L_textures[0]);
        if (b_HW_smap)
        {
            if (b_HW_PCF)
                C.r_Sampler_clf("s_smap", r2_RT_smap_depth);
            else
                C.r_Sampler_rtf("s_smap", r2_RT_smap_depth);
        }
        else
            C.r_Sampler_rtf("s_smap", r2_RT_smap_surf);
        jitter(C);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        C.r_End();
        break;
    case SE_L_TRANSLUENT: // shadowed + transluency
        C.r_Pass("accum_volume", "accum_omni_transluent", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler_clf("s_lmap", r2_RT_smap_surf); // diff here
        if (b_HW_smap)
        {
            if (b_HW_PCF)
                C.r_Sampler_clf("s_smap", r2_RT_smap_depth);
            else
                C.r_Sampler_rtf("s_smap", r2_RT_smap_depth);
        }
        else
            C.r_Sampler_rtf("s_smap", r2_RT_smap_surf);
        jitter(C);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        C.r_End();
        break;
    }
#elif RENDER == R_GL
    //BOOL	b_HW_smap		= RImplementation.o.HW_smap;
    //BOOL	b_HW_PCF		= RImplementation.o.HW_smap_PCF;
    BOOL blend = RImplementation.o.fp16_blend;
    D3DBLEND dest = blend ? D3DBLEND_ONE : D3DBLEND_ZERO;

    switch (C.iElement)
    {
    case SE_L_FILL: // fill projective
        //C.r_Pass			("null", 			"copy",						false,	FALSE,	FALSE);
        C.r_Pass("stub_notransform", "copy_nomsaa", false, FALSE, FALSE);
        C.r_Sampler("s_base", C.L_textures[0]);
        C.r_End();
        break;
    case SE_L_UNSHADOWED: // unshadowed
        C.r_Pass("accum_volume", "accum_omni_unshadowed_nomsaa", false, FALSE,FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler("s_lmap", C.L_textures[0]);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        C.r_End();
        break;
    case SE_L_NORMAL: // normal
        C.r_Pass("accum_volume", "accum_omni_normal_nomsaa", false, FALSE,FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler("s_lmap", C.L_textures[0]);
        C.r_Sampler_cmp("s_smap", r2_RT_smap_depth);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        jitter(C);
        C.r_End();
        break;
    case SE_L_FULLSIZE: // normal-fullsize
        C.r_Pass("accum_volume", "accum_omni_normal_nomsaa", false, FALSE,FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler("s_lmap", C.L_textures[0]);
        C.r_Sampler_cmp("s_smap", r2_RT_smap_depth);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        jitter(C);
        C.r_End();
        break;
    case SE_L_TRANSLUENT: // shadowed + transluency
        C.r_Pass("accum_volume", "accum_omni_transluent_nomsaa", false, FALSE,FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler_clf("s_lmap", r2_RT_smap_surf); // diff here
        C.r_Sampler_cmp("s_smap", r2_RT_smap_depth);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        jitter(C);
        C.r_End();
        break;
    }
#else
    // BOOL	b_HW_smap		= RImplementation.o.HW_smap;
    // BOOL	b_HW_PCF		= RImplementation.o.HW_smap_PCF;
    BOOL blend = RImplementation.o.fp16_blend;
    D3DBLEND dest = blend ? D3DBLEND_ONE : D3DBLEND_ZERO;
    switch (C.iElement)
    {
    case SE_L_FILL: // fill projective
        // C.r_Pass			("null", 			"copy",						false,	FALSE,	FALSE);
        C.r_Pass("stub_notransform", "copy_nomsaa", false, FALSE, FALSE);
        // C.r_Sampler			("s_base",			C.L_textures[0]	);
        C.r_dx11Texture("s_base", C.L_textures[0]);
        C.r_dx11Sampler("smp_nofilter");
        C.r_End();
        break;
    case SE_L_UNSHADOWED: // unshadowed
        C.r_Pass("accum_volume", "accum_omni_unshadowed_nomsaa", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        // C.r_Sampler_rtf		("s_position",		r2_RT_P);
        // C.r_Sampler_rtf		("s_normal",		r2_RT_N);
        // C.r_Sampler_clw		("s_material",		r2_material);
        // C.r_Sampler			("s_lmap",			C.L_textures[0]);
        // C.r_Sampler_rtf		("s_accumulator",	r2_RT_accum		);
        C.r_dx11Texture("s_position", r2_RT_P);
        C.r_dx11Texture("s_normal", r2_RT_N);
        C.r_dx11Texture("s_material", r2_material);
        C.r_dx11Texture("s_lmap", C.L_textures[0]);
        C.r_dx11Texture("s_accumulator", r2_RT_accum);
        C.r_dx11Texture("s_diffuse", r2_RT_albedo);

        C.r_dx11Sampler("smp_nofilter");
        C.r_dx11Sampler("smp_material");
        C.r_dx11Sampler("smp_rtlinear");
        C.r_End();
        break;
    case SE_L_NORMAL: // normal
        C.r_Pass("accum_volume", "accum_omni_normal_nomsaa", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        // C.r_Sampler_rtf		("s_position",		r2_RT_P);
        // C.r_Sampler_rtf		("s_normal",		r2_RT_N);
        // C.r_Sampler_clw		("s_material",		r2_material);
        // C.r_Sampler			("s_lmap",			C.L_textures[0]);
        // if (b_HW_smap)		{
        //	if (b_HW_PCF)	C.r_Sampler_clf		("s_smap",r2_RT_smap_depth	);
        //	else			C.r_Sampler_rtf		("s_smap",r2_RT_smap_depth	);
        //}
        // else				C.r_Sampler_rtf		("s_smap",r2_RT_smap_surf	);
        // jitter				(C);
        // C.r_Sampler_rtf		("s_accumulator",	r2_RT_accum		);
        C.r_dx11Texture("s_position", r2_RT_P);
        C.r_dx11Texture("s_normal", r2_RT_N);
        C.r_dx11Texture("s_material", r2_material);
        C.r_dx11Texture("s_lmap", C.L_textures[0]);
        C.r_dx11Texture("s_smap", r2_RT_smap_depth);
        C.r_dx11Texture("s_accumulator", r2_RT_accum);
        C.r_dx11Texture("s_diffuse", r2_RT_albedo);

        C.r_dx11Sampler("smp_nofilter");
        C.r_dx11Sampler("smp_material");
        C.r_dx11Sampler("smp_rtlinear");
        jitter(C);
        C.r_dx11Sampler("smp_smap");
        C.r_End();
        break;
    case SE_L_FULLSIZE: // normal-fullsize
        C.r_Pass("accum_volume", "accum_omni_normal_nomsaa", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        // C.r_Sampler_rtf		("s_position",		r2_RT_P);
        // C.r_Sampler_rtf		("s_normal",		r2_RT_N);
        // C.r_Sampler_clw		("s_material",		r2_material);
        // C.r_Sampler			("s_lmap",			C.L_textures[0]);
        // if (b_HW_smap)		{
        //	if (b_HW_PCF)	C.r_Sampler_clf		("s_smap",r2_RT_smap_depth	);
        //	else			C.r_Sampler_rtf		("s_smap",r2_RT_smap_depth	);
        //}
        // else				C.r_Sampler_rtf		("s_smap",r2_RT_smap_surf	);
        // jitter				(C);
        // C.r_Sampler_rtf		("s_accumulator",	r2_RT_accum		);
        C.r_dx11Texture("s_position", r2_RT_P);
        C.r_dx11Texture("s_normal", r2_RT_N);
        C.r_dx11Texture("s_material", r2_material);
        C.r_dx11Texture("s_lmap", C.L_textures[0]);
        C.r_dx11Texture("s_smap", r2_RT_smap_depth);
        C.r_dx11Texture("s_accumulator", r2_RT_accum);
        C.r_dx11Texture("s_diffuse", r2_RT_albedo);

        C.r_dx11Sampler("smp_nofilter");
        C.r_dx11Sampler("smp_material");
        C.r_dx11Sampler("smp_rtlinear");
        jitter(C);
        C.r_dx11Sampler("smp_smap");
        C.r_End();
        break;
    case SE_L_TRANSLUENT: // shadowed + transluency
        C.r_Pass("accum_volume", "accum_omni_transluent_nomsaa", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        // C.r_Sampler_rtf		("s_position",		r2_RT_P);
        // C.r_Sampler_rtf		("s_normal",		r2_RT_N);
        // C.r_Sampler_clw		("s_material",		r2_material);
        // C.r_Sampler_clf		("s_lmap",			r2_RT_smap_surf);			// diff here
        // if (b_HW_smap)		{
        //	if (b_HW_PCF)	C.r_Sampler_clf		("s_smap",r2_RT_smap_depth	);
        //	else			C.r_Sampler_rtf		("s_smap",r2_RT_smap_depth	);
        //}
        // else				C.r_Sampler_rtf		("s_smap",r2_RT_smap_surf	);
        // C.r_Sampler_rtf		("s_accumulator",	r2_RT_accum		);
        // jitter				(C);
        C.r_dx11Texture("s_position", r2_RT_P);
        C.r_dx11Texture("s_normal", r2_RT_N);
        C.r_dx11Texture("s_material", r2_material);
        C.r_dx11Texture("s_lmap", C.L_textures[0]);
        C.r_dx11Texture("s_smap", r2_RT_smap_depth);
        C.r_dx11Texture("s_accumulator", r2_RT_accum);
        C.r_dx11Texture("s_diffuse", r2_RT_albedo);

        C.r_dx11Sampler("smp_nofilter");
        C.r_dx11Sampler("smp_material");
        C.r_dx11Sampler("smp_rtlinear");
        jitter(C);
        C.r_dx11Sampler("smp_smap");
        C.r_End();
        break;
    }
#endif
}

#if RENDER != R_R2
void CBlender_accum_point_msaa::Compile(CBlender_Compile& C)
{
    IBlender::Compile(C);

    if (Name)
        GEnv.Render->m_MSAASample = atoi(Definition);
    else
        GEnv.Render->m_MSAASample = -1;

    // BOOL	b_HW_smap		= RImplementation.o.HW_smap;
    // BOOL	b_HW_PCF		= RImplementation.o.HW_smap_PCF;
    BOOL blend = RImplementation.o.fp16_blend;
    D3DBLEND dest = blend ? D3DBLEND_ONE : D3DBLEND_ZERO;
#if RENDER == R_GL
    switch (C.iElement)
    {
    case SE_L_FILL: // fill projective
        //C.r_Pass			("null", 			"copy",						false,	FALSE,	FALSE);
        C.r_Pass("stub_notransform", "copy_msaa", false, FALSE, FALSE);
        C.r_Sampler("s_base", C.L_textures[0]);
        C.r_End();
        break;
    case SE_L_UNSHADOWED: // unshadowed
        C.r_Pass("accum_volume", "accum_omni_unshadowed_msaa", false, FALSE,FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler("s_lmap", C.L_textures[0]);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        C.r_End();
        break;
    case SE_L_NORMAL: // normal
        C.r_Pass("accum_volume", "accum_omni_normal_msaa", false, FALSE,FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler("s_lmap", C.L_textures[0]);
        C.r_Sampler_cmp("s_smap", r2_RT_smap_depth);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        jitter(C);
        C.r_End();
        break;
    case SE_L_FULLSIZE: // normal-fullsize
        C.r_Pass("accum_volume", "accum_omni_normal_msaa", false, FALSE,FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler("s_lmap", C.L_textures[0]);
        C.r_Sampler_cmp("s_smap", r2_RT_smap_depth);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        jitter(C);
        C.r_End();
        break;
    case SE_L_TRANSLUENT: // shadowed + transluency
        C.r_Pass("accum_volume", "accum_omni_transluent_msaa", false, FALSE,FALSE, blend, D3DBLEND_ONE, dest);
        C.r_Sampler_rtf("s_position", r2_RT_P);
        C.r_Sampler_rtf("s_normal", r2_RT_N);
        C.r_Sampler_clw("s_material", r2_material);
        C.r_Sampler_clf("s_lmap", r2_RT_smap_surf); // diff here
        C.r_Sampler_cmp("s_smap", r2_RT_smap_depth);
        C.r_Sampler_rtf("s_accumulator", r2_RT_accum);
        jitter(C);
        C.r_End();
        break;
    }
#else
    switch (C.iElement)
    {
    case SE_L_FILL: // fill projective
        // C.r_Pass			("null", 			"copy",						false,	FALSE,	FALSE);
        C.r_Pass("stub_notransform", "copy_msaa", false, FALSE, FALSE);
        // C.r_Sampler			("s_base",			C.L_textures[0]	);
        C.r_dx11Texture("s_base", C.L_textures[0]);
        C.r_dx11Sampler("smp_nofilter");
        C.r_End();
        break;
    case SE_L_UNSHADOWED: // unshadowed
        C.r_Pass("accum_volume", "accum_omni_unshadowed_msaa", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        // C.r_Sampler_rtf		("s_position",		r2_RT_P);
        // C.r_Sampler_rtf		("s_normal",		r2_RT_N);
        // C.r_Sampler_clw		("s_material",		r2_material);
        // C.r_Sampler			("s_lmap",			C.L_textures[0]);
        // C.r_Sampler_rtf		("s_accumulator",	r2_RT_accum		);
        C.r_dx11Texture("s_position", r2_RT_P);
        C.r_dx11Texture("s_normal", r2_RT_N);
        C.r_dx11Texture("s_material", r2_material);
        C.r_dx11Texture("s_lmap", C.L_textures[0]);
        C.r_dx11Texture("s_accumulator", r2_RT_accum);
        C.r_dx11Texture("s_diffuse", r2_RT_albedo);

        C.r_dx11Sampler("smp_nofilter");
        C.r_dx11Sampler("smp_material");
        C.r_dx11Sampler("smp_rtlinear");
        C.r_End();
        break;
    case SE_L_NORMAL: // normal
        C.r_Pass("accum_volume", "accum_omni_normal_msaa", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        // C.r_Sampler_rtf		("s_position",		r2_RT_P);
        // C.r_Sampler_rtf		("s_normal",		r2_RT_N);
        // C.r_Sampler_clw		("s_material",		r2_material);
        // C.r_Sampler			("s_lmap",			C.L_textures[0]);
        // if (b_HW_smap)		{
        //	if (b_HW_PCF)	C.r_Sampler_clf		("s_smap",r2_RT_smap_depth	);
        //	else			C.r_Sampler_rtf		("s_smap",r2_RT_smap_depth	);
        //}
        // else				C.r_Sampler_rtf		("s_smap",r2_RT_smap_surf	);
        // jitter				(C);
        // C.r_Sampler_rtf		("s_accumulator",	r2_RT_accum		);
        C.r_dx11Texture("s_position", r2_RT_P);
        C.r_dx11Texture("s_normal", r2_RT_N);
        C.r_dx11Texture("s_material", r2_material);
        C.r_dx11Texture("s_lmap", C.L_textures[0]);
        C.r_dx11Texture("s_smap", r2_RT_smap_depth);
        C.r_dx11Texture("s_accumulator", r2_RT_accum);
        C.r_dx11Texture("s_diffuse", r2_RT_albedo);

        C.r_dx11Sampler("smp_nofilter");
        C.r_dx11Sampler("smp_material");
        C.r_dx11Sampler("smp_rtlinear");
        jitter(C);
        C.r_dx11Sampler("smp_smap");
        C.r_End();
        break;
    case SE_L_FULLSIZE: // normal-fullsize
        C.r_Pass("accum_volume", "accum_omni_normal_msaa", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        // C.r_Sampler_rtf		("s_position",		r2_RT_P);
        // C.r_Sampler_rtf		("s_normal",		r2_RT_N);
        // C.r_Sampler_clw		("s_material",		r2_material);
        // C.r_Sampler			("s_lmap",			C.L_textures[0]);
        // if (b_HW_smap)		{
        //	if (b_HW_PCF)	C.r_Sampler_clf		("s_smap",r2_RT_smap_depth	);
        //	else			C.r_Sampler_rtf		("s_smap",r2_RT_smap_depth	);
        //}
        // else				C.r_Sampler_rtf		("s_smap",r2_RT_smap_surf	);
        // jitter				(C);
        // C.r_Sampler_rtf		("s_accumulator",	r2_RT_accum		);
        C.r_dx11Texture("s_position", r2_RT_P);
        C.r_dx11Texture("s_normal", r2_RT_N);
        C.r_dx11Texture("s_material", r2_material);
        C.r_dx11Texture("s_lmap", C.L_textures[0]);
        C.r_dx11Texture("s_smap", r2_RT_smap_depth);
        C.r_dx11Texture("s_accumulator", r2_RT_accum);
        C.r_dx11Texture("s_diffuse", r2_RT_albedo);

        C.r_dx11Sampler("smp_nofilter");
        C.r_dx11Sampler("smp_material");
        C.r_dx11Sampler("smp_rtlinear");
        jitter(C);
        C.r_dx11Sampler("smp_smap");
        C.r_End();
        break;
    case SE_L_TRANSLUENT: // shadowed + transluency
        C.r_Pass("accum_volume", "accum_omni_transluent_msaa", false, FALSE, FALSE, blend, D3DBLEND_ONE, dest);
        // C.r_Sampler_rtf		("s_position",		r2_RT_P);
        // C.r_Sampler_rtf		("s_normal",		r2_RT_N);
        // C.r_Sampler_clw		("s_material",		r2_material);
        // C.r_Sampler_clf		("s_lmap",			r2_RT_smap_surf);			// diff here
        // if (b_HW_smap)		{
        //	if (b_HW_PCF)	C.r_Sampler_clf		("s_smap",r2_RT_smap_depth	);
        //	else			C.r_Sampler_rtf		("s_smap",r2_RT_smap_depth	);
        //}
        // else				C.r_Sampler_rtf		("s_smap",r2_RT_smap_surf	);
        // C.r_Sampler_rtf		("s_accumulator",	r2_RT_accum		);
        // jitter				(C);
        C.r_dx11Texture("s_position", r2_RT_P);
        C.r_dx11Texture("s_normal", r2_RT_N);
        C.r_dx11Texture("s_material", r2_material);
        C.r_dx11Texture("s_lmap", C.L_textures[0]);
        C.r_dx11Texture("s_smap", r2_RT_smap_depth);
        C.r_dx11Texture("s_accumulator", r2_RT_accum);
        C.r_dx11Texture("s_diffuse", r2_RT_albedo);

        C.r_dx11Sampler("smp_nofilter");
        C.r_dx11Sampler("smp_material");
        C.r_dx11Sampler("smp_rtlinear");
        jitter(C);
        C.r_dx11Sampler("smp_smap");
        C.r_End();
        break;
    }
#endif
    GEnv.Render->m_MSAASample = -1;
}
#endif
