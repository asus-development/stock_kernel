/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "clk: %s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>
#include <linux/msm-bus.h>
#include <dt-bindings/msm/msm-bus-ids.h>

#include "clk-debug.h"

#define MSM_BUS_VECTOR(_src, _dst, _ab, _ib)	\
{						\
	.src = _src,				\
	.dst = _dst,				\
	.ab = _ab,				\
	.ib = _ib,				\
}

static struct msm_bus_vectors clk_measure_vectors[] = {
	MSM_BUS_VECTOR(MSM_BUS_MASTER_AMPSS_M0,
			MSM_BUS_SLAVE_CAMERA_CFG, 0, 0),
	MSM_BUS_VECTOR(MSM_BUS_MASTER_AMPSS_M0,
			MSM_BUS_SLAVE_VENUS_CFG, 0, 0),
	MSM_BUS_VECTOR(MSM_BUS_MASTER_AMPSS_M0,
			MSM_BUS_SLAVE_DISPLAY_CFG, 0, 0),
	MSM_BUS_VECTOR(MSM_BUS_MASTER_AMPSS_M0,
			MSM_BUS_SLAVE_CAMERA_CFG, 0, 1),
	MSM_BUS_VECTOR(MSM_BUS_MASTER_AMPSS_M0,
			MSM_BUS_SLAVE_VENUS_CFG, 0, 1),
	MSM_BUS_VECTOR(MSM_BUS_MASTER_AMPSS_M0,
			MSM_BUS_SLAVE_DISPLAY_CFG, 0, 1),
};

static struct msm_bus_paths clk_measure_usecases[] = {
	{
		.num_paths = 3,
		.vectors = &clk_measure_vectors[0],
	},
	{
		.num_paths = 3,
		.vectors = &clk_measure_vectors[3],
	}
};

static struct msm_bus_scale_pdata clk_measure_scale_table = {
	.usecase = clk_measure_usecases,
	.num_usecases = ARRAY_SIZE(clk_measure_usecases),
	.name = "clk_measure",
};

static struct measure_clk_data debug_mux_priv = {
	.ctl_reg = 0x62038,
	.status_reg = 0x6203C,
	.xo_div4_cbcr = 0x43008,
};

static const char *const debug_mux_parent_names[] = {
	"cam_cc_bps_ahb_clk",
	"cam_cc_bps_areg_clk",
	"cam_cc_bps_axi_clk",
	"cam_cc_bps_clk",
	"cam_cc_camnoc_axi_clk",
	"cam_cc_camnoc_dcd_xo_clk",
	"cam_cc_cci_0_clk",
	"cam_cc_cci_1_clk",
	"cam_cc_core_ahb_clk",
	"cam_cc_cpas_ahb_clk",
	"cam_cc_csi0phytimer_clk",
	"cam_cc_csi1phytimer_clk",
	"cam_cc_csi2phytimer_clk",
	"cam_cc_csi3phytimer_clk",
	"cam_cc_csiphy0_clk",
	"cam_cc_csiphy1_clk",
	"cam_cc_csiphy2_clk",
	"cam_cc_csiphy3_clk",
	"cam_cc_fd_core_clk",
	"cam_cc_fd_core_uar_clk",
	"cam_cc_icp_ahb_clk",
	"cam_cc_icp_clk",
	"cam_cc_ife_0_axi_clk",
	"cam_cc_ife_0_clk",
	"cam_cc_ife_0_cphy_rx_clk",
	"cam_cc_ife_0_csid_clk",
	"cam_cc_ife_0_dsp_clk",
	"cam_cc_ife_1_axi_clk",
	"cam_cc_ife_1_clk",
	"cam_cc_ife_1_cphy_rx_clk",
	"cam_cc_ife_1_csid_clk",
	"cam_cc_ife_1_dsp_clk",
	"cam_cc_ife_lite_0_clk",
	"cam_cc_ife_lite_0_cphy_rx_clk",
	"cam_cc_ife_lite_0_csid_clk",
	"cam_cc_ife_lite_1_clk",
	"cam_cc_ife_lite_1_cphy_rx_clk",
	"cam_cc_ife_lite_1_csid_clk",
	"cam_cc_ipe_0_ahb_clk",
	"cam_cc_ipe_0_areg_clk",
	"cam_cc_ipe_0_axi_clk",
	"cam_cc_ipe_0_clk",
	"cam_cc_ipe_1_ahb_clk",
	"cam_cc_ipe_1_areg_clk",
	"cam_cc_ipe_1_axi_clk",
	"cam_cc_ipe_1_clk",
	"cam_cc_jpeg_clk",
	"cam_cc_lrme_clk",
	"cam_cc_mclk0_clk",
	"cam_cc_mclk1_clk",
	"cam_cc_mclk2_clk",
	"cam_cc_mclk3_clk",
	"disp_cc_mdss_ahb_clk",
	"disp_cc_mdss_byte0_clk",
	"disp_cc_mdss_byte0_intf_clk",
	"disp_cc_mdss_byte1_clk",
	"disp_cc_mdss_byte1_intf_clk",
	"disp_cc_mdss_dp_aux1_clk",
	"disp_cc_mdss_dp_aux_clk",
	"disp_cc_mdss_dp_crypto1_clk",
	"disp_cc_mdss_dp_crypto_clk",
	"disp_cc_mdss_dp_link1_clk",
	"disp_cc_mdss_dp_link1_intf_clk",
	"disp_cc_mdss_dp_link_clk",
	"disp_cc_mdss_dp_link_intf_clk",
	"disp_cc_mdss_dp_pixel1_clk",
	"disp_cc_mdss_dp_pixel2_clk",
	"disp_cc_mdss_dp_pixel_clk",
	"disp_cc_mdss_edp_aux_clk",
	"disp_cc_mdss_edp_gtc_clk",
	"disp_cc_mdss_edp_link_clk",
	"disp_cc_mdss_edp_link_intf_clk",
	"disp_cc_mdss_edp_pixel_clk",
	"disp_cc_mdss_esc0_clk",
	"disp_cc_mdss_esc1_clk",
	"disp_cc_mdss_mdp_clk",
	"disp_cc_mdss_mdp_lut_clk",
	"disp_cc_mdss_non_gdsc_ahb_clk",
	"disp_cc_mdss_pclk0_clk",
	"disp_cc_mdss_pclk1_clk",
	"disp_cc_mdss_rot_clk",
	"disp_cc_mdss_rscc_ahb_clk",
	"disp_cc_mdss_rscc_vsync_clk",
	"disp_cc_mdss_vsync_clk",
	"disp_cc_xo_clk",
	"measure_only_cdsp_clk",
	"measure_only_snoc_clk",
	"measure_only_cnoc_clk",
	"measure_only_mccc_clk",
	"measure_only_ipa_2x_clk",
	"gcc_aggre_noc_pcie_tbu_clk",
	"gcc_aggre_ufs_card_axi_clk",
	"gcc_aggre_ufs_phy_axi_clk",
	"gcc_aggre_usb3_prim_axi_clk",
	"gcc_aggre_usb3_sec_axi_clk",
	"gcc_camera_ahb_clk",
	"gcc_camera_hf_axi_clk",
	"gcc_camera_sf_axi_clk",
	"gcc_camera_xo_clk",
	"gcc_ce1_ahb_clk",
	"gcc_ce1_axi_clk",
	"gcc_ce1_clk",
	"gcc_cfg_noc_usb3_prim_axi_clk",
	"gcc_cfg_noc_usb3_sec_axi_clk",
	"gcc_cpuss_ahb_clk",
	"gcc_cpuss_rbcpr_clk",
	"gcc_ddrss_gpu_axi_clk",
	"gcc_disp_ahb_clk",
	"gcc_disp_hf_axi_clk",
	"gcc_disp_sf_axi_clk",
	"gcc_disp_xo_clk",
	"gcc_emac_axi_clk",
	"gcc_emac_ptp_clk",
	"gcc_emac_rgmii_clk",
	"gcc_emac_slv_ahb_clk",
	"gcc_gp1_clk",
	"gcc_gp2_clk",
	"gcc_gp3_clk",
	"gcc_gpu_cfg_ahb_clk",
	"gcc_gpu_gpll0_clk_src",
	"gcc_gpu_gpll0_div_clk_src",
	"gcc_gpu_memnoc_gfx_clk",
	"gcc_gpu_snoc_dvm_gfx_clk",
	"gcc_npu_at_clk",
	"gcc_npu_axi_clk",
	"gcc_npu_cfg_ahb_clk",
	"gcc_npu_gpll0_clk_src",
	"gcc_npu_gpll0_div_clk_src",
	"gcc_npu_trig_clk",
	"gcc_pcie0_phy_refgen_clk",
	"gcc_pcie1_phy_refgen_clk",
	"gcc_pcie_0_aux_clk",
	"gcc_pcie_0_cfg_ahb_clk",
	"gcc_pcie_0_mstr_axi_clk",
	"gcc_pcie_0_pipe_clk",
	"gcc_pcie_0_slv_axi_clk",
	"gcc_pcie_0_slv_q2a_axi_clk",
	"gcc_pcie_1_aux_clk",
	"gcc_pcie_1_cfg_ahb_clk",
	"gcc_pcie_1_mstr_axi_clk",
	"gcc_pcie_1_pipe_clk",
	"gcc_pcie_1_slv_axi_clk",
	"gcc_pcie_1_slv_q2a_axi_clk",
	"gcc_pcie_phy_aux_clk",
	"gcc_pdm2_clk",
	"gcc_pdm_ahb_clk",
	"gcc_pdm_xo4_clk",
	"gcc_prng_ahb_clk",
	"gcc_qspi_cnoc_periph_ahb_clk",
	"gcc_qspi_core_clk",
	"gcc_qupv3_wrap0_core_2x_clk",
	"gcc_qupv3_wrap0_core_clk",
	"gcc_qupv3_wrap0_s0_clk",
	"gcc_qupv3_wrap0_s1_clk",
	"gcc_qupv3_wrap0_s2_clk",
	"gcc_qupv3_wrap0_s3_clk",
	"gcc_qupv3_wrap0_s4_clk",
	"gcc_qupv3_wrap0_s5_clk",
	"gcc_qupv3_wrap0_s6_clk",
	"gcc_qupv3_wrap0_s7_clk",
	"gcc_qupv3_wrap1_core_2x_clk",
	"gcc_qupv3_wrap1_core_clk",
	"gcc_qupv3_wrap1_s0_clk",
	"gcc_qupv3_wrap1_s1_clk",
	"gcc_qupv3_wrap1_s2_clk",
	"gcc_qupv3_wrap1_s3_clk",
	"gcc_qupv3_wrap1_s4_clk",
	"gcc_qupv3_wrap1_s5_clk",
	"gcc_qupv3_wrap2_core_2x_clk",
	"gcc_qupv3_wrap2_core_clk",
	"gcc_qupv3_wrap2_s0_clk",
	"gcc_qupv3_wrap2_s1_clk",
	"gcc_qupv3_wrap2_s2_clk",
	"gcc_qupv3_wrap2_s3_clk",
	"gcc_qupv3_wrap2_s4_clk",
	"gcc_qupv3_wrap2_s5_clk",
	"gcc_sdcc2_ahb_clk",
	"gcc_sdcc2_apps_clk",
	"gcc_sdcc4_ahb_clk",
	"gcc_sdcc4_apps_clk",
	"gcc_sys_noc_cpuss_ahb_clk",
	"gcc_tsif_ahb_clk",
	"gcc_tsif_ref_clk",
	"gcc_ufs_card_ahb_clk",
	"gcc_ufs_card_axi_clk",
	"gcc_ufs_card_ice_core_clk",
	"gcc_ufs_card_phy_aux_clk",
	"gcc_ufs_card_rx_symbol_0_clk",
	"gcc_ufs_card_rx_symbol_1_clk",
	"gcc_ufs_card_tx_symbol_0_clk",
	"gcc_ufs_card_unipro_core_clk",
	"gcc_ufs_phy_ahb_clk",
	"gcc_ufs_phy_axi_clk",
	"gcc_ufs_phy_ice_core_clk",
	"gcc_ufs_phy_phy_aux_clk",
	"gcc_ufs_phy_rx_symbol_0_clk",
	"gcc_ufs_phy_rx_symbol_1_clk",
	"gcc_ufs_phy_tx_symbol_0_clk",
	"gcc_ufs_phy_unipro_core_clk",
	"gcc_usb30_prim_master_clk",
	"gcc_usb30_prim_mock_utmi_clk",
	"gcc_usb30_sec_master_clk",
	"gcc_usb30_sec_mock_utmi_clk",
	"gcc_usb3_prim_phy_aux_clk",
	"gcc_usb3_prim_phy_com_aux_clk",
	"gcc_usb3_prim_phy_pipe_clk",
	"gcc_usb3_sec_phy_aux_clk",
	"gcc_usb3_sec_phy_com_aux_clk",
	"gcc_usb3_sec_phy_pipe_clk",
	"gcc_video_ahb_clk",
	"gcc_video_axi0_clk",
	"gcc_video_axi1_clk",
	"gcc_video_axic_clk",
	"gcc_video_xo_clk",
	"gpu_cc_ahb_clk",
	"gpu_cc_cx_apb_clk",
	"gpu_cc_cx_gmu_clk",
	"gpu_cc_cx_qdss_at_clk",
	"gpu_cc_cx_qdss_trig_clk",
	"gpu_cc_cx_snoc_dvm_clk",
	"gpu_cc_cxo_aon_clk",
	"gpu_cc_cxo_clk",
	"gpu_cc_gx_gmu_clk",
	"gpu_cc_gx_vsense_clk",
	"measure_only_gpu_cc_cx_gfx3d_clk",
	"measure_only_gpu_cc_cx_gfx3d_slv_clk",
	"measure_only_gpu_cc_gx_gfx3d_clk",
	"npu_cc_armwic_core_clk",
	"npu_cc_bto_core_clk",
	"npu_cc_bwmon_clk",
	"npu_cc_cal_dp_cdc_clk",
	"npu_cc_cal_dp_clk",
	"npu_cc_comp_noc_axi_clk",
	"npu_cc_conf_noc_ahb_clk",
	"npu_cc_npu_core_apb_clk",
	"npu_cc_npu_core_atb_clk",
	"npu_cc_npu_core_clk",
	"npu_cc_npu_core_cti_clk",
	"npu_cc_npu_cpc_clk",
	"npu_cc_perf_cnt_clk",
	"npu_cc_xo_clk",
	"video_cc_iris_ahb_clk",
	"video_cc_mvs0_core_clk",
	"video_cc_mvs1_core_clk",
	"video_cc_mvsc_core_clk",
	"video_cc_xo_clk",
	"l3_clk",
	"pwrcl_clk",
	"perfcl_clk",
	"perfpcl_clk"
};

static struct clk_debug_mux gcc_debug_mux = {
	.priv = &debug_mux_priv,
	.debug_offset = 0x62000,
	.post_div_offset = 0x62004,
	.cbcr_offset = 0x62008,
	.src_sel_mask = 0x3FF,
	.src_sel_shift = 0,
	.post_div_mask = 0xF,
	.post_div_shift = 0,
	.period_offset = 0x50,
	MUX_SRC_LIST(
		{ "cam_cc_bps_ahb_clk", 0x55, 1, CAM_CC,
			0xE, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_bps_areg_clk", 0x55, 1, CAM_CC,
			0xD, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_bps_axi_clk", 0x55, 1, CAM_CC,
			0xC, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_bps_clk", 0x55, 1, CAM_CC,
			0xB, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_camnoc_axi_clk", 0x55, 1, CAM_CC,
			0x27, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_camnoc_dcd_xo_clk", 0x55, 1, CAM_CC,
			0x33, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_cci_0_clk", 0x55, 1, CAM_CC,
			0x2A, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_cci_1_clk", 0x55, 1, CAM_CC,
			0x3B, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_core_ahb_clk", 0x55, 1, CAM_CC,
			0x2E, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_cpas_ahb_clk", 0x55, 1, CAM_CC,
			0x2C, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_csi0phytimer_clk", 0x55, 1, CAM_CC,
			0x5, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_csi1phytimer_clk", 0x55, 1, CAM_CC,
			0x7, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_csi2phytimer_clk", 0x55, 1, CAM_CC,
			0x9, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_csi3phytimer_clk", 0x55, 1, CAM_CC,
			0x35, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_csiphy0_clk", 0x55, 1, CAM_CC,
			0x6, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_csiphy1_clk", 0x55, 1, CAM_CC,
			0x8, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_csiphy2_clk", 0x55, 1, CAM_CC,
			0xA, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_csiphy3_clk", 0x55, 1, CAM_CC,
			0x36, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_fd_core_clk", 0x55, 1, CAM_CC,
			0x28, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_fd_core_uar_clk", 0x55, 1, CAM_CC,
			0x29, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_icp_ahb_clk", 0x55, 1, CAM_CC,
			0x37, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_icp_clk", 0x55, 1, CAM_CC,
			0x26, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_0_axi_clk", 0x55, 1, CAM_CC,
			0x1B, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_0_clk", 0x55, 1, CAM_CC,
			0x17, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_0_cphy_rx_clk", 0x55, 1, CAM_CC,
			0x1A, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_0_csid_clk", 0x55, 1, CAM_CC,
			0x19, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_0_dsp_clk", 0x55, 1, CAM_CC,
			0x18, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_1_axi_clk", 0x55, 1, CAM_CC,
			0x21, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_1_clk", 0x55, 1, CAM_CC,
			0x1D, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_1_cphy_rx_clk", 0x55, 1, CAM_CC,
			0x20, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_1_csid_clk", 0x55, 1, CAM_CC,
			0x1F, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_1_dsp_clk", 0x55, 1, CAM_CC,
			0x1E, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_lite_0_clk", 0x55, 1, CAM_CC,
			0x22, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_lite_0_cphy_rx_clk", 0x55, 1, CAM_CC,
			0x24, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_lite_0_csid_clk", 0x55, 1, CAM_CC,
			0x23, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_lite_1_clk", 0x55, 1, CAM_CC,
			0x38, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_lite_1_cphy_rx_clk", 0x55, 1, CAM_CC,
			0x3A, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ife_lite_1_csid_clk", 0x55, 1, CAM_CC,
			0x39, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ipe_0_ahb_clk", 0x55, 1, CAM_CC,
			0x12, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ipe_0_areg_clk", 0x55, 1, CAM_CC,
			0x11, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ipe_0_axi_clk", 0x55, 1, CAM_CC,
			0x10, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ipe_0_clk", 0x55, 1, CAM_CC,
			0xF, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ipe_1_ahb_clk", 0x55, 1, CAM_CC,
			0x16, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ipe_1_areg_clk", 0x55, 1, CAM_CC,
			0x15, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ipe_1_axi_clk", 0x55, 1, CAM_CC,
			0x14, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_ipe_1_clk", 0x55, 1, CAM_CC,
			0x13, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_jpeg_clk", 0x55, 1, CAM_CC,
			0x25, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_lrme_clk", 0x55, 1, CAM_CC,
			0x2B, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_mclk0_clk", 0x55, 1, CAM_CC,
			0x1, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_mclk1_clk", 0x55, 1, CAM_CC,
			0x2, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_mclk2_clk", 0x55, 1, CAM_CC,
			0x3, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "cam_cc_mclk3_clk", 0x55, 1, CAM_CC,
			0x4, 0xFF, 0, 0xF, 0, 4, 0xD000, 0xD004, 0xD008 },
		{ "disp_cc_mdss_ahb_clk", 0x56, 1, DISP_CC,
			0x2B, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_byte0_clk", 0x56, 1, DISP_CC,
			0x15, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_byte0_intf_clk", 0x56, 1, DISP_CC,
			0x16, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_byte1_clk", 0x56, 1, DISP_CC,
			0x17, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_byte1_intf_clk", 0x56, 1, DISP_CC,
			0x18, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_aux1_clk", 0x56, 1, DISP_CC,
			0x25, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_aux_clk", 0x56, 1, DISP_CC,
			0x20, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_crypto1_clk", 0x56, 1, DISP_CC,
			0x24, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_crypto_clk", 0x56, 1, DISP_CC,
			0x1D, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_link1_clk", 0x56, 1, DISP_CC,
			0x22, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_link1_intf_clk", 0x56, 1, DISP_CC,
			0x23, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_link_clk", 0x56, 1, DISP_CC,
			0x1B, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_link_intf_clk", 0x56, 1, DISP_CC,
			0x1C, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_pixel1_clk", 0x56, 1, DISP_CC,
			0x1F, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_pixel2_clk", 0x56, 1, DISP_CC,
			0x21, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_dp_pixel_clk", 0x56, 1, DISP_CC,
			0x1E, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_edp_aux_clk", 0x56, 1, DISP_CC,
			0x29, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_edp_gtc_clk", 0x56, 1, DISP_CC,
			0x2A, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_edp_link_clk", 0x56, 1, DISP_CC,
			0x27, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_edp_link_intf_clk", 0x56, 1, DISP_CC,
			0x28, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_edp_pixel_clk", 0x56, 1, DISP_CC,
			0x26, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_esc0_clk", 0x56, 1, DISP_CC,
			0x19, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_esc1_clk", 0x56, 1, DISP_CC,
			0x1A, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_mdp_clk", 0x56, 1, DISP_CC,
			0x11, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_mdp_lut_clk", 0x56, 1, DISP_CC,
			0x13, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_non_gdsc_ahb_clk", 0x56, 1, DISP_CC,
			0x2C, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_pclk0_clk", 0x56, 1, DISP_CC,
			0xF, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_pclk1_clk", 0x56, 1, DISP_CC,
			0x10, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_rot_clk", 0x56, 1, DISP_CC,
			0x12, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_rscc_ahb_clk", 0x56, 1, DISP_CC,
			0x2E, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_rscc_vsync_clk", 0x56, 1, DISP_CC,
			0x2D, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_mdss_vsync_clk", 0x56, 1, DISP_CC,
			0x14, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "disp_cc_xo_clk", 0x56, 1, DISP_CC,
			0x36, 0xFF, 0, 0x3, 0, 4, 0x7000, 0x5008, 0x500C },
		{ "measure_only_cdsp_clk", 0xDB, 2, GCC,
			0xDB, 0x3FF, 0, 0xF, 0, 2, 0x62000, 0x62004, 0x62008 },
		{ "measure_only_snoc_clk", 0x7, 1, GCC,
			0x7, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "measure_only_cnoc_clk", 0x19, 1, GCC,
			0x19, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "measure_only_mccc_clk", 0xD0, 1, MC_CC,
			0xD0, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "measure_only_ipa_2x_clk", 0x147, 1, GCC,
			0x147, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_aggre_noc_pcie_tbu_clk", 0x36, 1, GCC,
			0x36, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_aggre_ufs_card_axi_clk", 0x141, 1, GCC,
			0x141, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_aggre_ufs_phy_axi_clk", 0x140, 1, GCC,
			0x140, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_aggre_usb3_prim_axi_clk", 0x13E, 1, GCC,
			0x13E, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_aggre_usb3_sec_axi_clk", 0x13F, 1, GCC,
			0x13F, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_camera_ahb_clk", 0x43, 1, GCC,
			0x43, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_camera_hf_axi_clk", 0x4D, 1, GCC,
			0x4D, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_camera_sf_axi_clk", 0x4E, 1, GCC,
			0x4E, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_camera_xo_clk", 0x52, 1, GCC,
			0x52, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ce1_ahb_clk", 0xB6, 1, GCC,
			0xB6, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ce1_axi_clk", 0xB5, 1, GCC,
			0xB5, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ce1_clk", 0xB4, 1, GCC,
			0xB4, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_cfg_noc_usb3_prim_axi_clk", 0x22, 1, GCC,
			0x22, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_cfg_noc_usb3_sec_axi_clk", 0x23, 1, GCC,
			0x23, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_cpuss_ahb_clk", 0xE0, 1, GCC,
			0xE0, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_cpuss_rbcpr_clk", 0xE2, 1, GCC,
			0xE2, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ddrss_gpu_axi_clk", 0xC0, 1, GCC,
			0xC0, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_disp_ahb_clk", 0x44, 1, GCC,
			0x44, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_disp_hf_axi_clk", 0x4F, 1, GCC,
			0x4F, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_disp_sf_axi_clk", 0x50, 1, GCC,
			0x50, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_disp_xo_clk", 0x53, 1, GCC,
			0x53, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_emac_axi_clk", 0x18D, 1, GCC,
			0x18D, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_emac_ptp_clk", 0x190, 1, GCC,
			0x190, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_emac_rgmii_clk", 0x18F, 1, GCC,
			0x18F, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_emac_slv_ahb_clk", 0x18E, 1, GCC,
			0x18E, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_gp1_clk", 0xF0, 1, GCC,
			0xF0, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_gp2_clk", 0xF1, 1, GCC,
			0xF1, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_gp3_clk", 0xF2, 1, GCC,
			0xF2, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_gpu_cfg_ahb_clk", 0x160, 1, GCC,
			0x160, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_gpu_gpll0_clk_src", 0x166, 1, GCC,
			0x166, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_gpu_gpll0_div_clk_src", 0x167, 1, GCC,
			0x167, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_gpu_memnoc_gfx_clk", 0x163, 1, GCC,
			0x163, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_gpu_snoc_dvm_gfx_clk", 0x165, 1, GCC,
			0x165, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_npu_at_clk", 0x17D, 1, GCC,
			0x17D, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_npu_axi_clk", 0x17B, 1, GCC,
			0x17B, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_npu_cfg_ahb_clk", 0x17A, 1, GCC,
			0x17A, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_npu_gpll0_clk_src", 0x17E, 1, GCC,
			0x17E, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_npu_gpll0_div_clk_src", 0x17F, 1, GCC,
			0x17F, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_npu_trig_clk", 0x17C, 1, GCC,
			0x17C, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie0_phy_refgen_clk", 0x104, 1, GCC,
			0x104, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie1_phy_refgen_clk", 0x105, 1, GCC,
			0x105, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_0_aux_clk", 0xF7, 1, GCC,
			0xF7, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_0_cfg_ahb_clk", 0xF6, 1, GCC,
			0xF6, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_0_mstr_axi_clk", 0xF5, 1, GCC,
			0xF5, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_0_pipe_clk", 0xF8, 1, GCC,
			0xF8, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_0_slv_axi_clk", 0xF4, 1, GCC,
			0xF4, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_0_slv_q2a_axi_clk", 0xF3, 1, GCC,
			0xF3, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_1_aux_clk", 0xFF, 1, GCC,
			0xFF, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_1_cfg_ahb_clk", 0xFE, 1, GCC,
			0xFE, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_1_mstr_axi_clk", 0xFD, 1, GCC,
			0xFD, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_1_pipe_clk", 0x100, 1, GCC,
			0x100, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_1_slv_axi_clk", 0xFC, 1, GCC,
			0xFC, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_1_slv_q2a_axi_clk", 0xFB, 1, GCC,
			0xFB, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pcie_phy_aux_clk", 0x103, 1, GCC,
			0x103, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pdm2_clk", 0x9A, 1, GCC,
			0x9A, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pdm_ahb_clk", 0x98, 1, GCC,
			0x98, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_pdm_xo4_clk", 0x99, 1, GCC,
			0x99, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_prng_ahb_clk", 0x9B, 1, GCC,
			0x9B, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qspi_cnoc_periph_ahb_clk", 0x178, 1, GCC,
			0x178, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qspi_core_clk", 0x179, 1, GCC,
			0x179, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_core_2x_clk", 0x85, 1, GCC,
			0x85, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_core_clk", 0x84, 1, GCC,
			0x84, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_s0_clk", 0x86, 1, GCC,
			0x86, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_s1_clk", 0x87, 1, GCC,
			0x87, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_s2_clk", 0x88, 1, GCC,
			0x88, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_s3_clk", 0x89, 1, GCC,
			0x89, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_s4_clk", 0x8A, 1, GCC,
			0x8A, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_s5_clk", 0x8B, 1, GCC,
			0x8B, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_s6_clk", 0x8C, 1, GCC,
			0x8C, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap0_s7_clk", 0x8D, 1, GCC,
			0x8D, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap1_core_2x_clk", 0x91, 1, GCC,
			0x91, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap1_core_clk", 0x90, 1, GCC,
			0x90, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap1_s0_clk", 0x92, 1, GCC,
			0x92, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap1_s1_clk", 0x93, 1, GCC,
			0x93, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap1_s2_clk", 0x94, 1, GCC,
			0x94, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap1_s3_clk", 0x95, 1, GCC,
			0x95, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap1_s4_clk", 0x96, 1, GCC,
			0x96, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap1_s5_clk", 0x97, 1, GCC,
			0x97, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap2_core_2x_clk", 0x184, 1, GCC,
			0x184, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap2_core_clk", 0x183, 1, GCC,
			0x183, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap2_s0_clk", 0x185, 1, GCC,
			0x185, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap2_s1_clk", 0x186, 1, GCC,
			0x186, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap2_s2_clk", 0x187, 1, GCC,
			0x187, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap2_s3_clk", 0x188, 1, GCC,
			0x188, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap2_s4_clk", 0x189, 1, GCC,
			0x189, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_qupv3_wrap2_s5_clk", 0x18A, 1, GCC,
			0x18A, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_sdcc2_ahb_clk", 0x7F, 1, GCC,
			0x7F, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_sdcc2_apps_clk", 0x7E, 1, GCC,
			0x7E, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_sdcc4_ahb_clk", 0x81, 1, GCC,
			0x81, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_sdcc4_apps_clk", 0x80, 1, GCC,
			0x80, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_sys_noc_cpuss_ahb_clk", 0xC, 1, GCC,
			0xC, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_tsif_ahb_clk", 0x9C, 1, GCC,
			0x9C, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_tsif_ref_clk", 0x9D, 1, GCC,
			0x9D, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_card_ahb_clk", 0x107, 1, GCC,
			0x107, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_card_axi_clk", 0x106, 1, GCC,
			0x106, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_card_ice_core_clk", 0x10D, 1, GCC,
			0x10D, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_card_phy_aux_clk", 0x10E, 1, GCC,
			0x10E, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_card_rx_symbol_0_clk", 0x109, 1, GCC,
			0x109, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_card_rx_symbol_1_clk", 0x10F, 1, GCC,
			0x10F, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_card_tx_symbol_0_clk", 0x108, 1, GCC,
			0x108, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_card_unipro_core_clk", 0x10C, 1, GCC,
			0x10C, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_phy_ahb_clk", 0x113, 1, GCC,
			0x113, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_phy_axi_clk", 0x112, 1, GCC,
			0x112, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_phy_ice_core_clk", 0x119, 1, GCC,
			0x119, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_phy_phy_aux_clk", 0x11A, 1, GCC,
			0x11A, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_phy_rx_symbol_0_clk", 0x115, 1, GCC,
			0x115, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_phy_rx_symbol_1_clk", 0x11B, 1, GCC,
			0x11B, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_phy_tx_symbol_0_clk", 0x114, 1, GCC,
			0x114, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_ufs_phy_unipro_core_clk", 0x118, 1, GCC,
			0x118, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb30_prim_master_clk", 0x6B, 1, GCC,
			0x6B, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb30_prim_mock_utmi_clk", 0x6D, 1, GCC,
			0x6D, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb30_sec_master_clk", 0x72, 1, GCC,
			0x72, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb30_sec_mock_utmi_clk", 0x74, 1, GCC,
			0x74, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb3_prim_phy_aux_clk", 0x6E, 1, GCC,
			0x6E, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb3_prim_phy_com_aux_clk", 0x6F, 1, GCC,
			0x6F, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb3_prim_phy_pipe_clk", 0x70, 1, GCC,
			0x70, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb3_sec_phy_aux_clk", 0x75, 1, GCC,
			0x75, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb3_sec_phy_com_aux_clk", 0x76, 1, GCC,
			0x76, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_usb3_sec_phy_pipe_clk", 0x77, 1, GCC,
			0x77, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_video_ahb_clk", 0x42, 1, GCC,
			0x42, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_video_axi0_clk", 0x4A, 1, GCC,
			0x4A, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_video_axi1_clk", 0x4B, 1, GCC,
			0x4B, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_video_axic_clk", 0x4C, 1, GCC,
			0x4C, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gcc_video_xo_clk", 0x51, 1, GCC,
			0x51, 0x3FF, 0, 0xF, 0, 1, 0x62000, 0x62004, 0x62008 },
		{ "gpu_cc_ahb_clk", 0x162, 1, GPU_CC,
			0x10, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "gpu_cc_cx_apb_clk", 0x162, 1, GPU_CC,
			0x14, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "gpu_cc_cx_gmu_clk", 0x162, 1, GPU_CC,
			0x18, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "gpu_cc_cx_qdss_at_clk", 0x162, 1, GPU_CC,
			0x12, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "gpu_cc_cx_qdss_trig_clk", 0x162, 1, GPU_CC,
			0x17, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "gpu_cc_cx_snoc_dvm_clk", 0x162, 1, GPU_CC,
			0x15, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "gpu_cc_cxo_aon_clk", 0x162, 1, GPU_CC,
			0xA, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "gpu_cc_cxo_clk", 0x162, 1, GPU_CC,
			0x19, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "gpu_cc_gx_gmu_clk", 0x162, 1, GPU_CC,
			0xF, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "gpu_cc_gx_vsense_clk", 0x162, 1, GPU_CC,
			0xC, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "measure_only_gpu_cc_cx_gfx3d_clk", 0x162, 1, GPU_CC,
			0x1A, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "measure_only_gpu_cc_cx_gfx3d_slv_clk", 0x162, 1, GPU_CC,
			0x1B, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "measure_only_gpu_cc_gx_gfx3d_clk", 0x162, 1, GPU_CC,
			0xB, 0xFF, 0, 0x3, 0, 2, 0x1568, 0x10FC, 0x1100 },
		{ "npu_cc_armwic_core_clk", 0x180, 1, NPU_CC,
			0x4, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_bto_core_clk", 0x180, 1, NPU_CC,
			0x12, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_bwmon_clk", 0x180, 1, NPU_CC,
			0xF, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_cal_dp_cdc_clk", 0x180, 1, NPU_CC,
			0x8, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_cal_dp_clk", 0x180, 1, NPU_CC,
			0x1, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_comp_noc_axi_clk", 0x180, 1, NPU_CC,
			0x9, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_conf_noc_ahb_clk", 0x180, 1, NPU_CC,
			0xA, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_npu_core_apb_clk", 0x180, 1, NPU_CC,
			0xE, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_npu_core_atb_clk", 0x180, 1, NPU_CC,
			0xB, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_npu_core_clk", 0x180, 1, NPU_CC,
			0x2, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_npu_core_cti_clk", 0x180, 1, NPU_CC,
			0xC, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_npu_cpc_clk", 0x180, 1, NPU_CC,
			0x3, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_perf_cnt_clk", 0x180, 1, NPU_CC,
			0x10, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "npu_cc_xo_clk", 0x180, 1, NPU_CC,
			0x11, 0xFF, 0, 0x3, 0, 2, 0x4000, 0x3004, 0x3008 },
		{ "video_cc_iris_ahb_clk", 0x57, 1, VIDEO_CC,
			0x7, 0x3F, 0, 0x7, 0, 5, 0xA4C, 0x938, 0x940 },
		{ "video_cc_mvs0_core_clk", 0x57, 1, VIDEO_CC,
			0x3, 0x3F, 0, 0x7, 0, 5, 0xA4C, 0x938, 0x940 },
		{ "video_cc_mvs1_core_clk", 0x57, 1, VIDEO_CC,
			0x5, 0x3F, 0, 0x7, 0, 5, 0xA4C, 0x938, 0x940 },
		{ "video_cc_mvsc_core_clk", 0x57, 1, VIDEO_CC,
			0x1, 0x3F, 0, 0x7, 0, 5, 0xA4C, 0x938, 0x940 },
		{ "video_cc_xo_clk", 0x57, 1, VIDEO_CC,
			0x8, 0x3F, 0, 0x7, 0, 5, 0xA4C, 0x938, 0x940 },
		{ "l3_clk", 0xE8, 4, CPU_CC,
			0x46, 0x7F, 4, 0xf, 11, 1, 0x0, 0x0, U32_MAX, 16 },
		{ "pwrcl_clk", 0xE8, 4, CPU_CC,
			0x44, 0x7F, 4, 0xf, 11, 1, 0x0, 0x0, U32_MAX, 16 },
		{ "perfcl_clk", 0xE8, 4, CPU_CC,
			0x45, 0x7F, 4, 0xf, 11, 1, 0x0, 0x0, U32_MAX, 16 },
		{ "perfpcl_clk", 0xE8, 4, CPU_CC,
			0x47, 0x7F, 4, 0xf, 11, 1, 0x0, 0x0, U32_MAX, 16 },
	),
	.hw.init = &(struct clk_init_data){
		.name = "gcc_debug_mux",
		.ops = &clk_debug_mux_ops,
		.parent_names = debug_mux_parent_names,
		.num_parents = ARRAY_SIZE(debug_mux_parent_names),
		.flags = CLK_IS_MEASURE,
	},
};

static const struct of_device_id clk_debug_match_table[] = {
	{ .compatible = "qcom,debugcc-sm8150" },
	{ }
};

static int map_debug_bases(struct platform_device *pdev, char *base, int cc)
{
	if (!of_get_property(pdev->dev.of_node, base, NULL))
		return -ENODEV;

	gcc_debug_mux.regmap[cc] =
			syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
					base);
	if (IS_ERR(gcc_debug_mux.regmap[cc])) {
		pr_err("Failed to map %s (ret=%ld)\n", base,
				PTR_ERR(gcc_debug_mux.regmap[cc]));
		return PTR_ERR(gcc_debug_mux.regmap[cc]);
	}
	return 0;
}

static int clk_debug_sm8150_probe(struct platform_device *pdev)
{
	struct clk *clk;
	int ret = 0;

	clk = devm_clk_get(&pdev->dev, "xo_clk_src");
	if (IS_ERR(clk)) {
		if (PTR_ERR(clk) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "Unable to get xo clock\n");
		return PTR_ERR(clk);
	}

	debug_mux_priv.cxo = clk;

	gcc_debug_mux.regmap = devm_kcalloc(&pdev->dev, MAX_NUM_CC,
				sizeof(*gcc_debug_mux.regmap), GFP_KERNEL);
	if (!gcc_debug_mux.regmap)
		return -ENOMEM;

	ret = map_debug_bases(pdev, "qcom,gcc", GCC);
	if (ret)
		return ret;

	ret = map_debug_bases(pdev, "qcom,dispcc", DISP_CC);
	if (ret)
		return ret;

	ret = map_debug_bases(pdev, "qcom,videocc", VIDEO_CC);
	if (ret)
		return ret;

	ret = map_debug_bases(pdev, "qcom,camcc", CAM_CC);
	if (ret)
		return ret;

	ret = map_debug_bases(pdev, "qcom,npucc", NPU_CC);
	if (ret)
		return ret;

	ret = map_debug_bases(pdev, "qcom,gpucc", GPU_CC);
	if (ret)
		return ret;

	ret = map_debug_bases(pdev, "qcom,cpucc", CPU_CC);
	if (ret)
		return ret;

	ret = map_debug_bases(pdev, "qcom,mccc", MC_CC);
	if (ret)
		return ret;

	gcc_debug_mux.bus_cl_id =
		msm_bus_scale_register_client(&clk_measure_scale_table);

	if (!gcc_debug_mux.bus_cl_id)
		return -EPROBE_DEFER;

	clk = devm_clk_register(&pdev->dev, &gcc_debug_mux.hw);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "Unable to register GCC debug mux\n");
		return PTR_ERR(clk);
	}

	ret = clk_debug_measure_register(&gcc_debug_mux.hw);
	if (ret)
		dev_err(&pdev->dev, "Could not register Measure clock\n");

	return ret;
}

static struct platform_driver clk_debug_driver = {
	.probe = clk_debug_sm8150_probe,
	.driver = {
		.name = "debugcc-sm8150",
		.of_match_table = clk_debug_match_table,
		.owner = THIS_MODULE,
	},
};

int __init clk_debug_sm8150_init(void)
{
	return platform_driver_register(&clk_debug_driver);
}
fs_initcall(clk_debug_sm8150_init);

MODULE_DESCRIPTION("QTI DEBUG CC SM8150 Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:debugcc-sm8150");
