[
  .eNB_config + .mac_stats | group_by(.bs_id) | map(add)[] | {
    bs_id: .bs_id,
    ues: [
      .LC.lcUeConfig + .ue_mac_stats | group_by(.rnti) | map(add)[] | {
        rnti: .rnti,
        conf: .sdap,
        stats: .mac_stats.sdapStats
      }
    ]
  }
]
