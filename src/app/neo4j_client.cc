/* The MIT License (MIT)

   Copyright (c) 2017 Shahab SHARIAT BAGHERI

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/


 
#include "neo4j_client.h"  

int x = 0;



void flexran::app::management::neo4j_client::run_periodic_task() {

::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());

for (auto& agent_id : agent_ids) {

// ::std::shared_ptr<enb_scheduling_info> enb_sched_info = get_scheduling_info(agent_id);
::std::shared_ptr<rib::enb_rib_info> agent_config = rib_.get_agent(agent_id);
protocol::flex_ue_config_reply& ue_configs = agent_config->get_ue_configs();

for (int UE_id = 0; UE_id < ue_configs.ue_config_size(); UE_id++) {
   protocol::flex_ue_config ue_config = ue_configs.ue_config(UE_id);

   /*TODO*/   

  }


}

if (x == 0){
    create_neo4j_graph();
    x++;
}

}


#ifdef __cplusplus
extern "C" {
#endif

#include <neo4j-client.h>



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
    char buf[128];
    printf("%s\n", neo4j_tostring(value, buf, sizeof(buf)));

    neo4j_close_results(results);
    neo4j_close(connection);
    neo4j_client_cleanup();

   
}



#ifdef __cplusplus
}
#endif

void flexran::app::management::neo4j_client::update_graph() {
  
  /*TODO*/
  

}

void flexran::app::neo4j_client::update_node(rib::subframe_t subframe){



  /*TODO*/
}


