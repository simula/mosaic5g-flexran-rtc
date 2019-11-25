[
  # add config and stats together in one object
  .eNB_config + .mac_stats | group_by(.bs_id) | map(add)[] | {
    bs_id: .bs_id,
    x2_ho_net_control: .eNB.cellConfig[0].x2HoNetControl,
    ues:  [
      # add UE config and UE stats together in one object
      .UE.ueConfig + .ue_mac_stats | group_by(.rnti) | map(add)[] | {
        rnti:       .rnti,
        imsi:       .imsi,                                            # from conf
        ueCategory: .capabilities.ueCategory,                         # from conf
        rrc_info:   .info,
        wbCqi:      .mac_stats.dlCqiReport.csiReport[0].p10csi.wbCqi, # from stats
        rrc_meas:   .mac_stats.rrcMeasurements,             # from stats
        gtp:        .mac_stats.gtpStats
      }
    ]
  }
]
