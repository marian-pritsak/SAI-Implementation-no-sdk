#include "mlnx_sai.h"
#include <fx_base_api.h>
#include <flextrum_types.h>

fx_handle_t fx_handle;
sx_acl_pbs_id_t router_pbs_id;
bool router_pbs_created;

typedef struct _mlnx_sai_ext_object_id_t {
    sai_object_type_t type;
    uint32_t offset;
} mlnx_sai_ext_object_id_t;

void mlnx_to_sai_ext_object_id(sai_object_id_t *entry_id, uint32_t offset, sai_object_type_t type)
{
    mlnx_sai_ext_object_id_t mlnx_object_id = {.type = type, .offset = offset};
    memcpy(entry_id, &mlnx_object_id, sizeof(mlnx_sai_ext_object_id_t));
}


sai_status_t sai_ext_oid_to_mlnx_offset(sai_object_id_t object_id, uint32_t *offset, sai_object_type_t expected_type)
{
    mlnx_sai_ext_object_id_t *mlnx_object_id = (mlnx_sai_ext_object_id_t *)&object_id;

    if (NULL == offset)
    {
        MLNX_SAI_LOG_ERR("NULL offset value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (expected_type != mlnx_object_id->type)
    {
        MLNX_SAI_LOG_ERR("Expected object %d got %d\n", expected_type, mlnx_object_id->type);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *offset = mlnx_object_id->offset;
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_create_table_peering_entry(
        _Out_ sai_object_id_t *entry_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    void *peering_keys[1];
    void *peering_params[1];
    uint16_t peer_offset = 0;
    uint16_t vnet_bitmap = 0;
    sx_port_log_id_t sx_log_port_id;
    flextrum_action_id_t peer_action_id;
    sai_status_t sai_status;
    uint32_t attr_idx;
    sai_object_id_t port_oid;
    const sai_attribute_value_t *attr;
    if (SAI_STATUS_SUCCESS ==
        (sai_status =
             find_attrib_in_list(attr_count, attr_list, SAI_TABLE_PEERING_ENTRY_ATTR_SRC_PORT, &attr, &attr_idx)))
    {
        if (SAI_STATUS_SUCCESS !=
            (sai_status = mlnx_object_to_type(attr->oid, SAI_OBJECT_TYPE_PORT, &sx_log_port_id, NULL)))
        {
            MLNX_SAI_LOG_ERR("Fail to get sx_port id from sai_port_id\n");
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_idx;
        }
        port_oid = attr->oid;
    }
    else
    {
        MLNX_SAI_LOG_ERR("Didn't recieve mandatory src port attribute\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS ==
        (sai_status =
             find_attrib_in_list(attr_count, attr_list, SAI_TABLE_PEERING_ENTRY_ATTR_META_REG, &attr, &attr_idx)))
    {
        vnet_bitmap = attr->u16;
    }
    else
    {
        MLNX_SAI_LOG_ERR("Didn't recieve mandatory meta reg attribute\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS ==
        (sai_status =
             find_attrib_in_list(attr_count, attr_list, SAI_TABLE_PEERING_ENTRY_ATTR_ACTION, &attr, &attr_idx)))
    {
        if (attr->s32 != SAI_TABLE_PEERING_ENTRY_ACTION_SET_VNET_BITMAP)
        {
            MLNX_SAI_LOG_ERR("Unsupported peering table action\n");
            return SAI_STATUS_NOT_IMPLEMENTED;
        }
        else
        {
            peer_action_id = SET_VNET_BITMAP_ID;
        }
    }
    else
    {
        MLNX_SAI_LOG_ERR("Didn't recieve mandatory peering action attribute\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    peering_keys[0] = (void *)&sx_log_port_id;
    peering_params[0] = (void *)&vnet_bitmap;
    sai_object_list_t in_port_if_list;
    in_port_if_list.count = 1;
    in_port_if_list.list = &port_oid;
    sai_ext_api_initialize(in_port_if_list);
    // if (add_table_entry_table_peering(peering_keys, NULL, peering_params,
    //                                       peer_action_id, &peer_offset))
    if (fx_table_entry_add(fx_handle, TABLE_PEERING_ID, peer_action_id, peering_keys, NULL, peering_params, &peer_offset))
    {
        MLNX_SAI_LOG_ERR("Failure in insertion of table_peering entry\n");
        return SAI_STATUS_FAILURE;
    }
    mlnx_to_sai_ext_object_id(entry_id, peer_offset, SAI_OBJECT_TYPE_TABLE_PEERING_ENTRY);
    // TODO update sai_object_id to encode offset and type
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_remove_table_peering_entry(
    _In_ sai_object_id_t entry_id)
{
    sai_status_t status;
    uint32_t peer_offset;
    if (SAI_STATUS_SUCCESS != (status = sai_ext_oid_to_mlnx_offset(entry_id, &peer_offset, SAI_OBJECT_TYPE_TABLE_PEERING_ENTRY)))
    {
        MLNX_SAI_LOG_ERR("Failure in extracting offest from peering entry object id 0x%" PRIx64 "\n", entry_id);
        return status;
    }
    if (fx_table_entry_remove(fx_handle, TABLE_PEERING_ID, peer_offset))
    {
        MLNX_SAI_LOG_ERR("Failure in removal of table_peering entry at offset %d\n", peer_offset);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_set_table_peering_entry_attribute(
    _In_ sai_object_id_t entry_id,
    _In_ const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_get_table_peering_entry_attribute(
    _In_ sai_object_id_t entry_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_create_table_vhost_entry(
    _Out_ sai_object_id_t *entry_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    uint16_t vnet_bitmap;
    uint16_t vnet_bitmap_mask;
    uint32_t overlay_dip;
    uint32_t underlay_dip;
    uint16_t vhost_offset;
    sx_acl_pbs_id_t port_pbs_id;
    flextrum_action_id_t vhost_action_id;
    sx_tunnel_id_t tunnel_id;
    uint32_t tunnel_idx;
    sai_status_t sai_status;
    uint32_t attr_idx;
    sx_port_log_id_t sx_log_port_id;
    sx_router_id_t vrid;
    sx_bridge_id_t bridge_id;
    const sai_attribute_value_t *attr;

    if (SAI_STATUS_SUCCESS ==
        (sai_status =
             find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_ACTION, &attr, &attr_idx)))
    {
        if (attr->s32 == SAI_TABLE_VHOST_ENTRY_ACTION_TO_TUNNEL) {
            vhost_action_id = TO_TUNNEL_ID;
            MLNX_SAI_LOG_NTC("vhost_actio_id %d (tunnel)\n", vhost_action_id);
        } else if (attr->s32 == SAI_TABLE_VHOST_ENTRY_ACTION_TO_PORT) {
            vhost_action_id = TO_PORT_ID;
            MLNX_SAI_LOG_NTC("vhost_actio_id %d (port)\n", vhost_action_id);
        }
        else if (attr->s32 == SAI_TABLE_VHOST_ENTRY_ACTION_TO_ROUTER)
        {
            vhost_action_id = TO_ROUTER_ID;
            MLNX_SAI_LOG_NTC("vhost_actio_id %d (router)\n", vhost_action_id);
        } else {
            MLNX_SAI_LOG_ERR("Unsupported action in vhost entry\n");
            return SAI_STATUS_NOT_IMPLEMENTED;
        }
    }
    else
    {
        MLNX_SAI_LOG_ERR("Didn't recieve mandatory vhost action attribute\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    void *vhost_keys[2];
    void *vhost_masks[1];
    void *vhost_params[3];
    bool default_entry = false;

    if (vhost_action_id == TO_TUNNEL_ID)
    {
        MLNX_SAI_LOG_NTC("inside tunnel. TO_TUNNEL_ID = %d\n", TO_TUNNEL_ID);
        if (SAI_STATUS_SUCCESS ==
            (sai_status =
                 find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_TUNNEL_ID, &attr, &attr_idx)))
        {
            if (SAI_STATUS_SUCCESS !=
                (sai_status = mlnx_object_to_type(attr->oid, SAI_OBJECT_TYPE_TUNNEL, &tunnel_idx, NULL)))
            {
                MLNX_SAI_LOG_ERR("Fail to get sx_port id from sai_port_id\n");
                return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_idx;
            }
            tunnel_id = g_sai_db_ptr->tunnel_db[tunnel_idx].sx_tunnel_id;
            printf("tunnel sai oid 0x%" PRIx64 ". tunnel mlnx oid 0x%x\n", attr->oid, (uint32_t)tunnel_id);
        }
        else
        {
            MLNX_SAI_LOG_ERR("Didn't recieve mandatory tunnel id attribute\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (SAI_STATUS_SUCCESS ==
            (sai_status =
                 find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_UNDERLAY_DIP, &attr, &attr_idx)))
        {
            underlay_dip = ntohl((uint32_t)attr->ipaddr.addr.ip4);
        }
        else
        {
            MLNX_SAI_LOG_ERR("Didn't recieve mandatory underlay dip attribute\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (SAI_STATUS_SUCCESS ==
            (sai_status = find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_BRIDGE_ID, &attr, &attr_idx)))
        {
            mlnx_object_id_t mlnx_bridge_id;
            sai_status = sai_to_mlnx_object_id(SAI_OBJECT_TYPE_BRIDGE, attr->oid, &mlnx_bridge_id);
            if (SAI_ERR(sai_status))
            {
                MLNX_SAI_LOG_ERR("Failed parse bridge id %" PRIx64 "\n", attr->oid);
            	return SAI_STATUS_INVALID_PARAMETER;
            }

            bridge_id = mlnx_bridge_id.id.bridge_id;
        }
        else
        {
            MLNX_SAI_LOG_ERR("Didn't recieve mandatory bridge id attribute\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        vhost_params[0] = (void *)&tunnel_id;
        vhost_params[1] = (void *)&underlay_dip;
        vhost_params[2] = (void *)&bridge_id;
    }

    if (vhost_action_id == TO_PORT_ID)
    {
        MLNX_SAI_LOG_NTC("inside port. TO_PORT_ID = %d\n", TO_PORT_ID);
        if (SAI_STATUS_SUCCESS ==
            (sai_status =
                 find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_PORT_ID, &attr, &attr_idx)))
        {
            if (SAI_STATUS_SUCCESS !=
                (sai_status = mlnx_object_to_type(attr->oid, SAI_OBJECT_TYPE_PORT, &sx_log_port_id, NULL)))
            {
                MLNX_SAI_LOG_ERR("Fail to get sx_port id from sai_port_id\n");
                return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_idx;
            }
            sx_acl_pbs_entry_t pbs_entry = {.entry_type = SX_ACL_PBS_ENTRY_TYPE_UNICAST, .port_num = 1, .log_ports = &sx_log_port_id};
            sx_status_t rc;
            rc = sx_api_acl_policy_based_switching_set(gh_sdk, SX_ACCESS_CMD_ADD, 0, &pbs_entry, &port_pbs_id);
            if (rc)
            {
                MLNX_SAI_LOG_ERR("Failure in pbs creation %d\n", rc);
                return SAI_STATUS_FAILURE;
            }
        }
        else
        {

            MLNX_SAI_LOG_ERR("Didn't recieve mandatory port id attribute\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        vhost_params[0] = (void *)&port_pbs_id;
    }

    if (vhost_action_id == TO_ROUTER_ID)
    {
        uint32_t data;
        MLNX_SAI_LOG_NTC("inside router. TO_ROUTER_ID = %d\n", TO_ROUTER_ID);
        if (SAI_STATUS_SUCCESS ==
            (sai_status =
                 find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_VR_ID, &attr, &attr_idx)))
        {
            if (SAI_STATUS_SUCCESS !=
                (sai_status = mlnx_object_to_type(attr->oid, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &data, NULL)))
            {
                MLNX_SAI_LOG_ERR("Fail to get vr id from sai_object_id\n");
                return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_idx;
            }
            vrid = (sx_router_id_t) data;
            if (router_pbs_created == false) {
                sx_acl_pbs_entry_t pbs_entry = {.entry_type = SX_ACL_PBS_ENTRY_TYPE_ROUTING, .port_num = 0, .log_ports = NULL};
                sx_status_t rc;
                rc = sx_api_acl_policy_based_switching_set(gh_sdk, SX_ACCESS_CMD_ADD, 0, &pbs_entry, &router_pbs_id);
                if (rc)
                {
                    MLNX_SAI_LOG_ERR("Failure in pbs creation %d. of vrid %d\n", rc, vrid);
                    return SAI_STATUS_FAILURE;
                }
                MLNX_SAI_LOG_NTC("Router PBS createdd\n");
                router_pbs_created = true;
            }
        }
        else
        {

            MLNX_SAI_LOG_ERR("Didn't recieve mandatory router id attribute\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        vhost_params[0] = (void *)&router_pbs_id;
    }

    if (SAI_STATUS_SUCCESS ==
        (sai_status =
             find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_IS_DEFAULT, &attr, &attr_idx)))
    {
        default_entry = attr->booldata; // TODO: currently priority == offset, need to implement offset managment
    }

    if (default_entry) {
        vhost_offset = 255; // TODO - get table siuze from base
        if (fx_table_entry_default_set(fx_handle, TABLE_VHOST_ID, vhost_action_id, vhost_params))
        {
            MLNX_SAI_LOG_ERR("Failure in insertion of table_vhost entry at offset %d\n", vhost_offset);
            return SAI_STATUS_FAILURE;
        }
        mlnx_to_sai_ext_object_id(entry_id, vhost_offset, SAI_OBJECT_TYPE_TABLE_VHOST_ENTRY);
    } else {
        if (SAI_STATUS_SUCCESS ==
            (sai_status =
                find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_PRIORITY, &attr, &attr_idx)))
        {
            vhost_offset = (uint16_t)attr->u32; // TODO: currently priority == offset, need to implement offset managment
            
        } else {
            MLNX_SAI_LOG_ERR("priority attribute not supported yet\n");
            return SAI_STATUS_NOT_IMPLEMENTED;
        }

        if (SAI_STATUS_SUCCESS ==
            (sai_status =
                find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_META_REG_KEY, &attr, &attr_idx)))
        {
            vnet_bitmap = attr->u16;
        }
        else
        {
            MLNX_SAI_LOG_ERR("Didn't recieve mandatory vnet bitmap key attribute\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }
        
        if (SAI_STATUS_SUCCESS ==
            (sai_status =
                find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_META_REG_MASK, &attr, &attr_idx)))
        {
            vnet_bitmap_mask = attr->u16;
        }
        else
        {
            MLNX_SAI_LOG_ERR("Didn't recieve mandatory vnet bitmap mask attribute\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (SAI_STATUS_SUCCESS ==
            (sai_status =
                find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_DST_IP, &attr, &attr_idx)))
        {
            overlay_dip = ntohl((uint32_t)attr->ipaddr.addr.ip4);
        }
        else
        {
            MLNX_SAI_LOG_ERR("Didn't recieve mandatory overlay dst ip attribute\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        vhost_keys[0] = (void *)&vnet_bitmap;
        vhost_masks[0] = (void *)&vnet_bitmap_mask;
        vhost_keys[1] = (void *)&overlay_dip;
        if (fx_table_entry_add(fx_handle, TABLE_VHOST_ID, vhost_action_id, vhost_keys, vhost_masks, vhost_params, &vhost_offset))
        {
            MLNX_SAI_LOG_ERR("Failure in insertion of table_vhost entry at offset %d\n", vhost_offset);
            return SAI_STATUS_FAILURE;
        }
        mlnx_to_sai_ext_object_id(entry_id, vhost_offset, SAI_OBJECT_TYPE_TABLE_VHOST_ENTRY);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_remove_table_vhost_entry(
    _In_ sai_object_id_t entry_id)
{
    sai_status_t status;
    uint32_t vhost_offset;
    if (SAI_STATUS_SUCCESS != (status = sai_ext_oid_to_mlnx_offset(entry_id, &vhost_offset, SAI_OBJECT_TYPE_TABLE_VHOST_ENTRY)))
    {
        MLNX_SAI_LOG_ERR("Failure in extracting offest from vhost entry object id 0x%" PRIx64 "\n", entry_id);
        return status;
    }
    if (fx_table_entry_remove(fx_handle, TABLE_VHOST_ID, vhost_offset))
    {
        MLNX_SAI_LOG_ERR("Failure in removal of table_vhost entry at offset %d\n", vhost_offset);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_set_table_vhost_entry_attribute(
    _In_ sai_object_id_t entry_id,
    _In_ const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_get_table_vhost_entry_attribute(
    _In_ sai_object_id_t entry_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_get_bmtor_stats(sai_object_id_t entry_id, uint32_t number_of_counters, const sai_bmtor_stat_t *counter_ids, uint64_t *counters) {
    sai_status_t status;
    uint32_t offset;
    sai_object_type_t object_type = SAI_OBJECT_TYPE_NULL;
    flextrum_table_id_t table_id = TABLE_PEERING_ID;
    uint32_t i;
    for (i = 0; i < number_of_counters; i++) {
        if ((counter_ids[i] == SAI_BMTOR_STAT_TABLE_PEERING_HIT_PACKETS) || (counter_ids[i] == SAI_BMTOR_STAT_TABLE_PEERING_HIT_OCTETS)) {
            if (object_type == SAI_OBJECT_TYPE_TABLE_VHOST_ENTRY) {
                MLNX_SAI_LOG_ERR("Got mixed counters of different objects.");
                return SAI_STATUS_INVALID_PARAMETER;
            }
            object_type = SAI_OBJECT_TYPE_TABLE_PEERING_ENTRY;
            table_id = TABLE_PEERING_ID;
        }
        if ((counter_ids[i] == SAI_BMTOR_STAT_TABLE_VHOST_HIT_PACKETS) || (counter_ids[i] == SAI_BMTOR_STAT_TABLE_VHOST_HIT_OCTETS)) {
            if (object_type == SAI_OBJECT_TYPE_TABLE_PEERING_ENTRY) {
                MLNX_SAI_LOG_ERR("Got mixed counters of different objects.");
                return SAI_STATUS_INVALID_PARAMETER;
            }
            object_type = SAI_OBJECT_TYPE_TABLE_VHOST_ENTRY;
            table_id = TABLE_VHOST_ID;
        }
    }
    if (SAI_STATUS_SUCCESS != (status = sai_ext_oid_to_mlnx_offset(entry_id, &offset, object_type)))
    {
        MLNX_SAI_LOG_ERR("Failure in extracting offest from entry object id 0x%" PRIx64 "\n", entry_id);
        return status;
    }
    uint64_t bytes;
    uint64_t packets;
    sx_status_t rc = fx_table_rule_counter_read(fx_handle, table_id, offset, &bytes, &packets);
    if (rc != SX_STATUS_SUCCESS) {
        MLNX_SAI_LOG_ERR("Failure in reading counters from entry object id 0x%" PRIx64 " (offset %d) \n", entry_id, offset);
        return status;
    }

    for (i = 0; i < number_of_counters; i++) {
        if ((counter_ids[i] == SAI_BMTOR_STAT_TABLE_PEERING_HIT_PACKETS) || (counter_ids[i] == SAI_BMTOR_STAT_TABLE_VHOST_HIT_PACKETS)) {
            counters[i] = packets;
        }
        if ((counter_ids[i] == SAI_BMTOR_STAT_TABLE_VHOST_HIT_OCTETS) || (counter_ids[i] == SAI_BMTOR_STAT_TABLE_PEERING_HIT_OCTETS)) {
            counters[i] = bytes;
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_clear_bmtor_stats(sai_object_id_t entry_id, uint32_t number_of_counters, const sai_bmtor_stat_t *counter_ids) {
    sai_status_t status;
    uint32_t offset;
    sai_object_type_t object_type = SAI_OBJECT_TYPE_NULL;
    flextrum_table_id_t table_id = TABLE_PEERING_ID;
    uint32_t i;
    for (i = 0; i < number_of_counters; i++) {
        if ((counter_ids[i] == SAI_BMTOR_STAT_TABLE_PEERING_HIT_PACKETS) || (counter_ids[i] == SAI_BMTOR_STAT_TABLE_PEERING_HIT_OCTETS)) {
            if (object_type == SAI_OBJECT_TYPE_TABLE_VHOST_ENTRY) {
                MLNX_SAI_LOG_ERR("Got mixed counters of different objects.");
                return SAI_STATUS_INVALID_PARAMETER;
            }
            object_type = SAI_OBJECT_TYPE_TABLE_PEERING_ENTRY;
            table_id = TABLE_PEERING_ID;
        }
        if ((counter_ids[i] == SAI_BMTOR_STAT_TABLE_VHOST_HIT_PACKETS) || (counter_ids[i] == SAI_BMTOR_STAT_TABLE_VHOST_HIT_OCTETS)) {
            if (object_type == SAI_OBJECT_TYPE_TABLE_PEERING_ENTRY) {
                MLNX_SAI_LOG_ERR("Got mixed counters of different objects.");
                return SAI_STATUS_INVALID_PARAMETER;
            }
            object_type = SAI_OBJECT_TYPE_TABLE_VHOST_ENTRY;
            table_id = TABLE_VHOST_ID;
        }
    }
    if (SAI_STATUS_SUCCESS != (status = sai_ext_oid_to_mlnx_offset(entry_id, &offset, object_type)))
    {
        MLNX_SAI_LOG_ERR("Failure in extracting offest from entry object id 0x%" PRIx64 "\n", entry_id);
        return status;
    }

    sx_status_t rc = fx_table_rule_counter_clear(fx_handle, table_id, offset);
    if (rc != SX_STATUS_SUCCESS) {
        MLNX_SAI_LOG_ERR("Failure in clearing counters from entry object id 0x%" PRIx64 " (offset %d) \n", entry_id, offset);
        return status;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ext_api_initialize(sai_object_list_t in_port_if_list) {
    int num_of_ports = in_port_if_list.count;
    sx_port_log_id_t *port_list = (sx_port_log_id_t *)malloc(sizeof(sx_port_log_id_t) * num_of_ports);
    sai_status_t sai_status;
    int i;
    for (i = 0; i < num_of_ports; i++)
    {
        if (SAI_STATUS_SUCCESS !=
            (sai_status = mlnx_object_to_type(in_port_if_list.list[i], SAI_OBJECT_TYPE_PORT, &port_list[i], NULL)))
        {
            MLNX_SAI_LOG_ERR("Fail to get sx_port id from sai_port_id 0x%" PRIx64 "\n", in_port_if_list.list[i]);
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }
    }
    fx_init(&fx_handle);
    fx_extern_init(fx_handle);
    sx_status_t rc = fx_pipe_create(fx_handle, FX_IN_PORT, (void *)port_list, num_of_ports);
    if (rc)
    {
        printf("Error - rc:%d\n", rc);
        return rc;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ext_api_uninitialize(sai_object_list_t in_port_if_list) {
    int num_of_ports = in_port_if_list.count;
    sx_port_log_id_t *port_list = (sx_port_log_id_t*) malloc(sizeof(sx_port_log_id_t) * num_of_ports);
    sai_status_t sai_status;
    int i;
    for (i=0; i<num_of_ports; i++) {
        if (SAI_STATUS_SUCCESS !=
            (sai_status = mlnx_object_to_type(in_port_if_list.list[i], SAI_OBJECT_TYPE_PORT, &port_list[i], NULL)))
        {
            MLNX_SAI_LOG_ERR("Fail to get sx_port id from sai_port_id 0x%" PRIx64 "\n", in_port_if_list.list[i]);
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }
    }
    fx_pipe_destroy(fx_handle, FX_IN_PORT, (void *) port_list, num_of_ports);
    fx_extern_deinit(fx_handle);
    fx_deinit(fx_handle);
    return SAI_STATUS_SUCCESS;
}

const sai_bmtor_api_t mlnx_bmtor_api = {
    mlnx_create_table_peering_entry,
    mlnx_remove_table_peering_entry,
    mlnx_set_table_peering_entry_attribute,
    mlnx_get_table_peering_entry_attribute,
    mlnx_create_table_vhost_entry,
    mlnx_remove_table_vhost_entry,
    mlnx_set_table_vhost_entry_attribute,
    mlnx_get_table_vhost_entry_attribute,
    mlnx_get_bmtor_stats,
    mlnx_clear_bmtor_stats
};
