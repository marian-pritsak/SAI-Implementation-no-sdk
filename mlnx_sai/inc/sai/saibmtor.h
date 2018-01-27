/* *
 * @file    saibmtor.h
 *
 * @brief   This module defines SAI || P4 extension  interface
 */

#if !defined (__SAIBMTOR_H_)
#define __SAIBMTOR_H_

#include <saitypes.h>

    /**
 * @defgroup SAIBMTOR SAI - Extension specific API definitions
 *
 * @{
 */

    /**
 * @brief Attribute data for #SAI_TABLE_PEERING_ENTRY_ATTR_ACTION
 */
typedef enum _sai_table_peering_entry_action_t {
    SAI_TABLE_PEERING_ENTRY_ACTION_SET_VNET_BITMAP,

    SAI_TABLE_PEERING_ENTRY_ACTION_NOACTION,

} sai_table_peering_entry_action_t;

/**
 * @brief Attribute data for #SAI_TABLE_VHOST_ENTRY_ATTR_ACTION
 */
typedef enum _sai_table_vhost_entry_action_t
{
    SAI_TABLE_VHOST_ENTRY_ACTION_TO_TUNNEL,

    SAI_TABLE_VHOST_ENTRY_ACTION_NOACTION,

} sai_table_vhost_entry_action_t;

/**
 * @brief Attribute ID for table_peering
 */
typedef enum _sai_table_peering_entry_attr_t
{
    /**
     * @brief Start of attributes
     */
    SAI_TABLE_PEERING_ENTRY_ATTR_START,

    /**
     * @brief Action
     *
     * @type sai_table_peering_entry_action_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    SAI_TABLE_PEERING_ENTRY_ATTR_ACTION = SAI_TABLE_PEERING_ENTRY_ATTR_START,

    /**
     * @brief Matched key src_port
     *
     * @type sai_object_id_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     * @objects SAI_OBJECT_TYPE_PORT
     */
    SAI_TABLE_PEERING_ENTRY_ATTR_SRC_PORT,

    /**
     * @brief Action set_vnet_bitmap parameter meta_reg
     *
     * @type sai_uint16_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     * @isvlan false
     * @condition SAI_TABLE_PEERING_ENTRY_ATTR_ACTION == SAI_TABLE_PEERING_ENTRY_ACTION_SET_VNET_BITMAP
     */
    SAI_TABLE_PEERING_ENTRY_ATTR_META_REG,

    /**
     * @brief End of attributes
     */
    SAI_TABLE_PEERING_ENTRY_ATTR_END,

    /** Custom range base value */
    SAI_TABLE_PEERING_ENTRY_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** End of custom range base */
    SAI_TABLE_PEERING_ENTRY_ATTR_CUSTOM_RANGE_END,

} sai_table_peering_entry_attr_t;

/**
 * @brief Attribute ID for table_vhost
 */
typedef enum _sai_table_vhost_entry_attr_t
{
    /**
     * @brief Start of attributes
     */
    SAI_TABLE_VHOST_ENTRY_ATTR_START,

    /**
     * @brief Action
     *
     * @type sai_table_vhost_entry_action_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    SAI_TABLE_VHOST_ENTRY_ATTR_ACTION = SAI_TABLE_VHOST_ENTRY_ATTR_START,

    /**
     * @brief Rule priority in table
     *
     * @type sai_uint32_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    SAI_TABLE_VHOST_ENTRY_ATTR_PRIORITY,

    /**
     * @brief Matched key meta_reg (key)
     *
     * @type sai_uint16_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     * @isvlan false
     */
    SAI_TABLE_VHOST_ENTRY_ATTR_META_REG_KEY,

    /**
     * @brief Matched key meta_reg (mask)
     *
     * @type sai_uint16_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     * @isvlan false
     */
    SAI_TABLE_VHOST_ENTRY_ATTR_META_REG_MASK,

    /**
     * @brief Matched key dst_ip
     *
     * @type sai_ip_address_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    SAI_TABLE_VHOST_ENTRY_ATTR_DST_IP,

    /**
     * @brief Action to_tunnel parameter tunnel_id
     *
     * @type sai_object_id_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     * @objects SAI_OBJECT_TYPE_TUNNEL
     * @condition SAI_TABLE_VHOST_ENTRY_ATTR_ACTION == SAI_TABLE_VHOST_ENTRY_ACTION_TO_TUNNEL
     */
    SAI_TABLE_VHOST_ENTRY_ATTR_TUNNEL_ID,

    /**
     * @brief Action to_tunnel parameter underlay_dip
     *
     * @type sai_ip_address_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     * @condition SAI_TABLE_VHOST_ENTRY_ATTR_ACTION == SAI_TABLE_VHOST_ENTRY_ACTION_TO_TUNNEL
     */
    SAI_TABLE_VHOST_ENTRY_ATTR_UNDERLAY_DIP,

    /**
     * @brief End of attributes
     */
    SAI_TABLE_VHOST_ENTRY_ATTR_END,

    /** Custom range base value */
    SAI_TABLE_VHOST_ENTRY_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** End of custom range base */
    SAI_TABLE_VHOST_ENTRY_ATTR_CUSTOM_RANGE_END,

} sai_table_vhost_entry_attr_t;

/**
 * @brief Create table_peering_entry
 *
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 * @param[out] entry_id Entry id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t(*sai_create_table_peering_entry_fn)(
        _Out_ sai_object_id_t *entry_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list);

/**
 * @brief Remove table_peering_entry
 *
 * @param[in] entry_id Entry id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t(*sai_remove_table_peering_entry_fn)(
        _In_ sai_object_id_t entry_id);

/**
 * @brief Set attribute for table_peering_entry
 *
 * @param[in] entry_id Entry id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t(*sai_set_table_peering_entry_attribute_fn)(
        _In_ sai_object_id_t entry_id,
        _In_ const sai_attribute_t *attr);

/**
 * @brief Get attribute for table_peering_entry
 *
 * @param[in] entry_id Entry id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t(*sai_get_table_peering_entry_attribute_fn)(
        _In_ sai_object_id_t entry_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list);

/**
 * @brief Create table_vhost_entry
 *
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 * @param[out] entry_id Entry id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t(*sai_create_table_vhost_entry_fn)(
        _Out_ sai_object_id_t *entry_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list);

/**
 * @brief Remove table_vhost_entry
 *
 * @param[in] entry_id Entry id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t(*sai_remove_table_vhost_entry_fn)(
        _In_ sai_object_id_t entry_id);

/**
 * @brief Set attribute for table_vhost_entry
 *
 * @param[in] entry_id Entry id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t(*sai_set_table_vhost_entry_attribute_fn)(
        _In_ sai_object_id_t entry_id,
        _In_ const sai_attribute_t *attr);

/**
 * @brief Get attribute for table_vhost_entry
 *
 * @param[in] entry_id Entry id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
typedef sai_status_t(*sai_get_table_vhost_entry_attribute_fn)(
        _In_ sai_object_id_t entry_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list);

typedef struct _sai_bmtor_api_t
{
    sai_create_table_peering_entry_fn            create_table_peering_entry;
    sai_remove_table_peering_entry_fn            remove_table_peering_entry;
    sai_set_table_peering_entry_attribute_fn    set_table_peering_entry_attribute;
    sai_get_table_peering_entry_attribute_fn    get_table_peering_entry_attribute;
    sai_create_table_vhost_entry_fn            create_table_vhost_entry;
    sai_remove_table_vhost_entry_fn            remove_table_vhost_entry;
    sai_set_table_vhost_entry_attribute_fn    set_table_vhost_entry_attribute;
    sai_get_table_vhost_entry_attribute_fn    get_table_vhost_entry_attribute;
} sai_bmtor_api_t;
/**
 * @}
 */
#endif /** __SAIBMTOR_H_ */