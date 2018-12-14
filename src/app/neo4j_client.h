/*
 * Copyright 2016-2018 FlexRAN Authors, Eurecom and The University of Edinburgh
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * For more information about Mosaic5G:  contact@mosaic-5g.io
 */

/*! \file    neo4j_client.h
 *  \brief   Neo4J app for graph database connection
 *  \authors Shahab SHARIAT BAGHERI
 *  \company Eurecom
 *  \email   shahab.shariat@eurecom.fr
 */

#ifndef NEO4J_CLIENT_H_
#define NEO4J_CLIENT_H_

#include <map>
#include <memory>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "component.h"
#include "enb_rib_info.h"
// #include "component.h"
// #include "ue_scheduling_info.h"
#include "enb_scheduling_info.h"
#include "rib_common.h"
#include "flexran.pb.h"

namespace flexran {

  namespace app {

    namespace management {

      class neo4j_client : public component {
	
      public:	
	
        neo4j_client(const rib::Rib& rib, const core::requests_manager& rm,
            event::subscription& sub)
          : component(rib, rm, sub) {}

	void update_graph();
	
	void update_node(rib::subframe_t subframe);

	void create_neo4j_graph();
    
        void periodic_task();

    // void neo4j_register();

	// void update_edge(rib::rnti_t rnti);
		
	private:

	int y = 0;
  int x = 0;
  int num_UEs_now = 0;
  int num_UEs_pre = 0;	

    };


   }
   

  }

}
      
#endif
