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

/*! \file    neo4j_client.cc
 *  \brief   Neo4J app for graph database connection
 *  \authors Shahab SHARIAT BAGHERI
 *  \company Eurecom
 *  \email   shahab.shariat@eurecom.fr
 */

 
#include "neo4j_client.h"  
#include "rt_controller_common.h"




#ifdef __cplusplus
extern "C" {
#endif

#include <neo4j-client.h>


#ifdef __cplusplus
}
#endif


void flexran::app::management::neo4j_client::periodic_task() {


if (y == 0){
    create_neo4j_graph();
    y++;
}


  for (uint64_t bs_id : rib_.get_available_base_stations()) {

   std::shared_ptr<rib::enb_rib_info> bs_config = rib_.get_bs(bs_id);
   const protocol::flex_ue_config_reply& ue_configs = bs_config->get_ue_configs();



   num_UEs_now = ue_configs.ue_config_size();

   if ((num_UEs_now - num_UEs_pre) > 0){


    neo4j_connection_t *connection =
            neo4j_connect("neo4j://neo4j:flexran@localhost:7687", NULL, NEO4J_INSECURE);
    if (connection == NULL)
    {
        neo4j_perror(stderr, errno, "Connection failed");
        exit(1);
    }


   neo4j_result_stream_t *results = neo4j_run(connection, "MATCH (n:Person) WHERE n.name = 'EnodeB' CREATE (u:Person { name: 'UE'}), (n)-[:CONN]->(u)", neo4j_null);
  

    neo4j_result_t *result = neo4j_fetch_next(results);
    if (results == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to fetch result");
        exit(1);
    }

     neo4j_value_t value = neo4j_result_field(result, 0);
     _unused(value);
     // char buf[128]; // Test purpose
    // printf("%s\n", neo4j_tostring(value, buf, sizeof(buf)));


   } else if ((num_UEs_now - num_UEs_pre) < 0){


    neo4j_connection_t *connection =
            neo4j_connect("neo4j://neo4j:flexran@localhost:7687", NULL, NEO4J_INSECURE);
    if (connection == NULL)
    {
        neo4j_perror(stderr, errno, "Connection failed");
        exit(1);
    }

   neo4j_result_stream_t *results =
            neo4j_run(connection, "MATCH (n)-[rel:CONN]->(r) WHERE n.name='EnodeB' AND r.name='UE' DELETE rel", neo4j_null);

    neo4j_result_stream_t *r = neo4j_run(connection, "MATCH (n:Person) WHERE n.name='UE' DELETE n", neo4j_null);
    _unused(r);
    
    neo4j_result_t *result = neo4j_fetch_next(results);
    if (results == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to fetch result");
        exit(1);
    }

     neo4j_value_t value = neo4j_result_field(result, 0);
     _unused(value);
     // char buf[128]; // Test purpose
    // printf("%s\n", neo4j_tostring(value, buf, sizeof(buf)));




   }

   num_UEs_pre = num_UEs_now;



}


  x++;  

  if (x == 1000){

    x = 0;
  }
  

}


#ifdef __cplusplus
extern "C" {
#endif


void flexran::app::management::neo4j_client::create_neo4j_graph(){

   neo4j_client_init();

    /* use NEO4J_INSECURE when connecting to disable TLS */
    neo4j_connection_t *connection =
            neo4j_connect("neo4j://neo4j:flexran@localhost:7687", NULL, NEO4J_INSECURE);
    if (connection == NULL)
    {
        neo4j_perror(stderr, errno, "Connection failed");
        exit(1);
    }

    neo4j_result_stream_t *results =
            neo4j_run(connection, "CREATE (n:Person { name: 'EnodeB'}) RETURN n", neo4j_null);
    if (results == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to run statement");
        exit(1);
    }

    neo4j_result_t *result = neo4j_fetch_next(results);
    if (results == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to fetch result");
        exit(1);
    }

    neo4j_value_t value = neo4j_result_field(result, 0);
    _unused(value);
    // char buf[128];
    // printf("%s\n", neo4j_tostring(value, buf, sizeof(buf)));

    neo4j_close_results(results);
    neo4j_close(connection);
    neo4j_client_cleanup();

   
}



#ifdef __cplusplus
}
#endif



// void flexran::app::management::neo4j_client::neo4j_register(rib::frame_t current_subframe) {


// ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());

// for (auto& agent_id : agent_ids) {

// ::std::shared_ptr<rib::enb_rib_info> agent_config = rib_.get_agent(agent_id);
// protocol::flex_ue_config_reply& ue_configs = agent_config->get_ue_configs();

// num_UEs_pre = ue_configs.ue_config_size();

// }



// }   


void flexran::app::management::neo4j_client::update_graph() {
  
  /*TODO*/
  

}

void flexran::app::management::neo4j_client::update_node(rib::subframe_t subframe){

  _unused(subframe);

  /*TODO*/
}


