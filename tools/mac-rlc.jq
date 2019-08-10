[
  .mac_stats[] | {
    bs_id: .bs_id,
    ues:  .ue_mac_stats[] | {
      rnti:       .rnti,
      rlc:        [
        .mac_stats.rlcReport[] | {
          lcId: .lcId,
          bytes: .txQueueSize
        }
      ],
      mac: .mac_stats.macStats,
    }
  }
]
