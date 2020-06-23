{
  slices: .eNB_config?[0].eNB.cellConfig[0]?.sliceConfig,
  UEs: [
    .eNB_config?[0].UE.ueConfig[]? | {
      rnti: .rnti,
      imsi: .imsi,
      dlSliceId: .dlSliceId,
      ulSliceId: .ulSliceId
    }
  ]
}
