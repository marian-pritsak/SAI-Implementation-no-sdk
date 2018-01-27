#include "mlnx_sai.h"
#include <fx_base_api.h>
#include <flextrum_types.h>

fx_handle_t fx_handle;

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
    printf("mlnx_create_table_peering_entry\n");
    void *peering_keys[1];
    void *peering_params[1];
    uint16_t peer_offset = 0;
    uint16_t vnet_bitmap = 0;
    sx_port_log_id_t sx_log_port_id;
    flextrum_action_id_t peer_action_id;
    sai_status_t sai_status;
    uint32_t attr_idx;
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
    // if (add_table_entry_table_peering(peering_keys, NULL, peering_params,
    //                                       peer_action_id, &peer_offset))
    if (fx_table_entry_add(fx_handle, TABLE_PEERING_ID, peer_action_id, peering_keys, NULL, peering_params, &peer_offset))
    {
        MLNX_SAI_LOG_ERR("Failure in insertion of table_peering entry\n");
        return SAI_STATUS_FAILURE;
    }
    printf("peering entry added at offset %d\n", peer_offset);
    mlnx_to_sai_ext_object_id(entry_id, peer_offset, SAI_OBJECT_TYPE_TABLE_PEERING_ENTRY);
    // TODO update sai_object_id to encode offset and type
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_remove_table_peering_entry(
    _In_ sai_object_id_t entry_id)
{
    printf("mlnx_remove_table_peering_entry\n");
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
    printf("mlnx_set_table_peering_entry_attribute\n");
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_get_table_peering_entry_attribute(
    _In_ sai_object_id_t entry_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    printf("mlnx_get_table_peering_entry_attribute\n");
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_create_table_vhost_entry(
    _Out_ sai_object_id_t *entry_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    printf("mlnx_create_table_vhost_entry\n");
    uint16_t vnet_bitmap;
    uint16_t vnet_bitmap_mask;
    uint32_t overlay_dip;
    uint32_t underlay_dip;
    uint16_t vhost_offset;
    flextrum_action_id_t vhost_action_id;
    sx_tunnel_id_t tunnel_id;
    uint32_t tunnel_idx;
    sai_status_t sai_status;
    uint32_t attr_idx;
    const sai_attribute_value_t *attr;

    if (SAI_STATUS_SUCCESS ==
        (sai_status =
             find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_ACTION, &attr, &attr_idx)))
    {
        if (attr->s32 != SAI_TABLE_VHOST_ENTRY_ACTION_TO_TUNNEL)
        {
            MLNX_SAI_LOG_ERR("Unsupported vhost table action\n");
            return SAI_STATUS_NOT_IMPLEMENTED;
        }
        else
        {
            vhost_action_id = TO_TUNNEL_ID;
        }
    }
    else
    {
        MLNX_SAI_LOG_ERR("Didn't recieve mandatory vhost action attribute\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

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
             find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_TUNNEL_ID, &attr, &attr_idx)))
    {
        if (SAI_STATUS_SUCCESS !=
            (sai_status = mlnx_object_to_type(attr->oid, SAI_OBJECT_TYPE_TUNNEL, &tunnel_idx, NULL)))
        {
            MLNX_SAI_LOG_ERR("Fail to get sx_port id from sai_port_id\n");
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_idx;
        }
        tunnel_id = g_sai_db_ptr->tunnel_db[tunnel_idx].sx_tunnel_id;
        printf("tunnel sai oid 0x%" PRIx64 ". tunnel mlnx oid 0x%x\n", attr->oid, (uint32_t) tunnel_id);
    }
    else
    {
        MLNX_SAI_LOG_ERR("Didn't recieve mandatory tunnel id attribute\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS ==
        (sai_status =
             find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_DST_IP, &attr, &attr_idx)))
    {
        overlay_dip = ntohl((uint32_t) attr->ipaddr.addr.ip4);
    }
    else
    {
        MLNX_SAI_LOG_ERR("Didn't recieve mandatory overlay dst ip attribute\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS ==
        (sai_status =
             find_attrib_in_list(attr_count, attr_list, SAI_TABLE_VHOST_ENTRY_ATTR_UNDERLAY_DIP, &attr, &attr_idx)))
    {
        underlay_dip = ntohl((uint32_t) attr->ipaddr.addr.ip4);
    }
    else
    {
        MLNX_SAI_LOG_ERR("Didn't recieve mandatory underlay dip attribute\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    void *vhost_keys[2];
    void *vhost_masks[1];
    void *vhost_params[2];
    vhost_keys[0] =   (void *)&vnet_bitmap;
    vhost_masks[0] =  (void *)&vnet_bitmap_mask;
    vhost_keys[1] =   (void *)&overlay_dip;
    vhost_params[0] = (void *)&tunnel_id;
    vhost_params[1] = (void *)&underlay_dip;
    printf("key 0: 0x%x, mask0: 0x%x, key 1: 0x%x, param0: 0x%x, param1: 0x%x\n", *((uint16_t *)vhost_keys[0]), *((uint16_t *)vhost_masks[0]), *((uint32_t *)vhost_keys[1]), *((uint32_t *)vhost_params[0]), *((uint32_t *)vhost_params[1]));
    if (fx_table_entry_add(fx_handle, TABLE_VHOST_ID, vhost_action_id, vhost_keys, vhost_masks, vhost_params, &vhost_offset))
    {
        MLNX_SAI_LOG_ERR("Failure in insertion of table_vhost entry at offset %d\n", vhost_offset);
        return SAI_STATUS_FAILURE;
    }
    printf("vhost entry added at offset %d\n", vhost_offset);
    mlnx_to_sai_ext_object_id(entry_id, vhost_offset, SAI_OBJECT_TYPE_TABLE_VHOST_ENTRY);
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_remove_table_vhost_entry(
    _In_ sai_object_id_t entry_id)
{
    printf("mlnx_remove_table_vhost_entry\n");
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
    printf("mlnx_set_table_vhost_entry_attribute\n");
    return SAI_STATUS_SUCCESS;
}

sai_status_t mlnx_get_table_vhost_entry_attribute(
    _In_ sai_object_id_t entry_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    printf("mlnx_get_table_vhost_entry_attribute\n");
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_extension_api_initialize() {    
    sx_port_log_id_t port_list[PORT_NUM];
    uint32_t num_of_ports = PORT_NUM;
    fx_init(&fx_handle);
    fx_extern_init(fx_handle);
    sx_status_t rc = fx_get_bindable_port_list(fx_handle, port_list, &num_of_ports);
    if (rc)
    {
        printf("Error - rc:%d\n", rc);
        return rc;
    }
    rc = fx_pipe_create(fx_handle, FX_IN_PORT, (void *)port_list, num_of_ports);
    if (rc)
    {
        printf("Error - rc:%d\n", rc);
        return rc;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_extension_api_uninitialize()
{
    sx_port_log_id_t port_list[PORT_NUM];
    uint32_t num_of_ports = PORT_NUM;
    sx_status_t rc = fx_get_bindable_port_list(fx_handle, port_list, &num_of_ports);
    if (rc)
    {
        printf("Error - rc:%d\n", rc);
        return rc;
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
};
