#!/bin/bash

host=localhost
port=9999
interval=1

watch -n$interval "curl -sX GET http://$host:$port/stats | jq '
{
  slices: {
    dl: [
      .eNB_config?[0].eNB.cellConfig[0]?.sliceConfig.dl[]? | {
        id: .id,
        label: .label,
        percentage: .percentage,
        maxmcs: .maxmcs
      }
    ],
    ul: [
      .eNB_config?[0].eNB.cellConfig[0]?.sliceConfig.ul[]? | {
        id: .id,
        label: .label,
        percentage: .percentage,
        maxmcs: .maxmcs
      }
    ]
  },
  UEs: [
    .eNB_config?[0].UE.ueConfig[]? | {
      rnti: .rnti,
      imsi: .imsi,
      dlSliceId: .dlSliceId,
      ulSliceId: .ulSliceId
    }
  ]
}
'"
