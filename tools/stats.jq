[
  .mac_stats[] | {
    bs: .bs_id,
    ues: [
      .ue_mac_stats[] | {
        rnti: .rnti,
        phr: .mac_stats.phr,
        dlWbCqi: .mac_stats.dlCqiReport.csiReport[0].p10csi.wbCqi,
        macTxBytes: .mac_stats.macStats.totalBytesSdusDl,
        macTxPrb: .mac_stats.macStats.totalPrbDl,
        macRxBytes: .mac_stats.macStats.totalBytesSdusUl,
        macRxPrb: .mac_stats.macStats.totalPrbUl,
        pdcpTx: .mac_stats.pdcpStats.pktTx,
        pdcpTxBytes: .mac_stats.pdcpStats.pktTxBytes,
        pdcpRx: .mac_stats.pdcpStats.pktRx,
        pdcpRxBytes: .mac_stats.pdcpStats.pktRxBytes,
      }
    ]
  }
]
