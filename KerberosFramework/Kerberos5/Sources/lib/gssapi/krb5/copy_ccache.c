/* -*- mode: c; indent-tabs-mode: nil -*- */
#include "gssapiP_krb5.h"

OM_uint32 KRB5_CALLCONV
gss_krb5int_copy_ccache(minor_status, cred_handle, out_ccache)
    OM_uint32 *minor_status;
    gss_cred_id_t cred_handle;
    krb5_ccache out_ccache;
{
    OM_uint32 major_status;
    krb5_gss_cred_id_t k5creds;
    krb5_cc_cursor cursor;
    krb5_creds creds;
    krb5_error_code code;
    krb5_context context;

    /* validate the cred handle */
    major_status = krb5_gss_validate_cred(minor_status, cred_handle);
    if (major_status)
        return(major_status);

    k5creds = (krb5_gss_cred_id_t) cred_handle;
    code = k5_mutex_lock(&k5creds->lock);
    if (code) {
        *minor_status = code;
        return GSS_S_FAILURE;
    }
    if (k5creds->usage == GSS_C_ACCEPT) {
        k5_mutex_unlock(&k5creds->lock);
        *minor_status = (OM_uint32) G_BAD_USAGE;
        return(GSS_S_FAILURE);
    }

    code = krb5_gss_init_context(&context);
    if (code) {
        k5_mutex_unlock(&k5creds->lock);
        *minor_status = code;
        return GSS_S_FAILURE;
    }

    code = krb5_cc_start_seq_get(context, k5creds->ccache, &cursor);
    if (code) {
        k5_mutex_unlock(&k5creds->lock);
        *minor_status = code;
        save_error_info(*minor_status, context);
        krb5_free_context(context);
        return(GSS_S_FAILURE);
    }
    while (!code && !krb5_cc_next_cred(context, k5creds->ccache, &cursor, &creds))
        code = krb5_cc_store_cred(context, out_ccache, &creds);
    krb5_cc_end_seq_get(context, k5creds->ccache, &cursor);
    k5_mutex_unlock(&k5creds->lock);
    *minor_status = code;
    if (code)
        save_error_info(*minor_status, context);
    krb5_free_context(context);
    return code ? GSS_S_FAILURE : GSS_S_COMPLETE;
}