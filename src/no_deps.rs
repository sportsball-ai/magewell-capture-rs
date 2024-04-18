/**
 *
 * This module contains stubs for third-party dependencies of the Magewell Capture SDK.
 * The SDK requires these dependencies to be present in the build. However, none of them are
 * necessary to perform capture from PCIe devices, so for a more minimal build, we can eliminate
 * the dependencies by stubbing out the essential symbols of these libraries.
 *
 * It is impossible for the vast majority of these stubs to be called, but we make our best effort
 * to always return a valid value or error code anyway.
 *
 */

// Creates a stub with the given return type and constant return value.
macro_rules! return_expr {
    ($name:ident, $ret:ty, $val:expr) => {
        #[no_mangle]
        pub extern "C" fn $name() -> $ret {
            // Want to see when this is called? Uncomment the following line.
            //println!("{} called", stringify!($name));
            $val
        }
    };
}

// Creates a stub that always returns null.
macro_rules! return_null {
    ($name:ident) => {
        #[no_mangle]
        pub extern "C" fn $name() -> *mut std::ffi::c_void {
            // Want to see when this is called? Uncomment the following line.
            //println!("{} called", stringify!($name));
            std::ptr::null_mut()
        }
    };
}

// Creates a stub with no return value.
macro_rules! return_void {
    ($name:ident) => {
        #[no_mangle]
        pub extern "C" fn $name() {
            // Want to see when this is called? Uncomment the following line.
            //println!("{} called", stringify!($name));
        }
    };
}

// Creates a stub that always returns a negative system error code.
macro_rules! return_error {
    ($name:ident) => {
        #[no_mangle]
        pub extern "C" fn $name() -> std::ffi::c_int {
            // Want to see when this is called? Uncomment the following line.
            //println!("{} called", stringify!($name));
            -(nix::errno::Errno::ENOENT as std::ffi::c_int)
        }
    };
}

mod no_v4l2 {
    use std::ffi::c_void;

    return_error!(v4l2_open);
    return_error!(v4l2_close);
    return_error!(v4l2_ioctl);
    return_expr!(v4l2_mmap, *mut c_void, -1_isize as *mut c_void);
    return_error!(v4l2_munmap);
}

mod no_udev {
    return_null!(udev_new);
    return_null!(udev_unref);
    return_null!(udev_enumerate_new);
    return_null!(udev_enumerate_unref);
    return_error!(udev_enumerate_add_match_subsystem);
    return_error!(udev_enumerate_add_match_property);
    return_error!(udev_enumerate_scan_devices);
    return_null!(udev_enumerate_get_list_entry);
    return_null!(udev_list_entry_get_next);
    return_null!(udev_list_entry_get_name);
    return_null!(udev_device_new_from_syspath);
    return_null!(udev_device_unref);
    return_null!(udev_device_get_devnode);
    return_null!(udev_device_get_parent_with_subsystem_devtype);
    return_null!(udev_device_get_sysname);
    return_null!(udev_device_get_sysattr_value);
    return_null!(udev_device_get_action);
    return_null!(udev_monitor_new_from_netlink);
    return_null!(udev_monitor_unref);
    return_error!(udev_monitor_enable_receiving);
    return_null!(udev_monitor_receive_device);
    return_error!(udev_monitor_get_fd);
    return_error!(udev_monitor_filter_add_match_subsystem_devtype);
}

mod no_usb {
    use std::ffi::c_int;

    const NOT_SUPPORTED: c_int = 12;

    return_expr!(libusb_attach_kernel_driver, c_int, NOT_SUPPORTED);
    return_expr!(libusb_claim_interface, c_int, NOT_SUPPORTED);
    return_void!(libusb_close);
    return_expr!(libusb_control_transfer, c_int, NOT_SUPPORTED);
    return_expr!(libusb_detach_kernel_driver, c_int, NOT_SUPPORTED);
    return_void!(libusb_exit);
    return_void!(libusb_free_device_list);
    return_expr!(libusb_get_bus_number, u8, 0);
    return_expr!(libusb_get_device_address, u8, 0);
    return_expr!(libusb_get_device_descriptor, c_int, NOT_SUPPORTED);
    return_expr!(libusb_get_device_list, isize, 0);
    return_expr!(libusb_get_port_number, u8, 0);
    return_expr!(libusb_get_string_descriptor_ascii, c_int, NOT_SUPPORTED);
    return_expr!(libusb_handle_events_timeout_completed, c_int, NOT_SUPPORTED);
    return_expr!(libusb_hotplug_register_callback, c_int, NOT_SUPPORTED);
    return_expr!(libusb_init, c_int, NOT_SUPPORTED);
    return_expr!(libusb_kernel_driver_active, c_int, NOT_SUPPORTED);
    return_expr!(libusb_open, c_int, NOT_SUPPORTED);
    return_expr!(libusb_release_interface, c_int, NOT_SUPPORTED);
}

mod no_asound {
    use std::ffi::{c_int, c_long};

    return_error!(snd_card_next);
    return_error!(snd_config_update_free_global);
    return_error!(snd_ctl_card_info);
    return_null!(snd_ctl_card_info_get_id);
    return_error!(snd_ctl_card_info_get_name);
    return_expr!(snd_ctl_card_info_sizeof, usize, 16 /* arbitrary */);
    return_error!(snd_ctl_close);
    return_error!(snd_ctl_open);
    return_error!(snd_mixer_attach);
    return_error!(snd_mixer_close);
    return_error!(snd_mixer_detach);
    return_expr!(snd_mixer_elem_get_type, c_int, 0 /* simple */);
    return_null!(snd_mixer_elem_next);
    return_null!(snd_mixer_first_elem);
    return_error!(snd_mixer_load);
    return_error!(snd_mixer_open);
    return_error!(snd_mixer_selem_get_capture_switch);
    return_error!(snd_mixer_selem_get_capture_volume);
    return_error!(snd_mixer_selem_get_capture_volume_range);
    return_null!(snd_mixer_selem_get_name);
    return_expr!(snd_mixer_selem_has_capture_channel, c_int, 0);
    return_expr!(snd_mixer_selem_has_capture_volume, c_int, 0);
    return_expr!(snd_mixer_selem_has_playback_volume, c_int, 0);
    return_expr!(snd_mixer_selem_is_active, c_int, 0);
    return_error!(snd_mixer_selem_register);
    return_error!(snd_mixer_selem_set_capture_dB);
    return_error!(snd_mixer_selem_set_capture_switch);
    return_error!(snd_mixer_selem_set_capture_switch_all);
    return_error!(snd_mixer_selem_set_capture_volume);
    return_error!(snd_mixer_selem_set_playback_dB);
    return_error!(snd_mixer_selem_set_playback_switch_all);
    return_expr!(snd_pcm_avail_update, c_long, -1);
    return_error!(snd_pcm_close);
    return_error!(snd_pcm_drain);
    return_error!(snd_pcm_drop);
    return_error!(snd_pcm_hw_params);
    return_error!(snd_pcm_hw_params_any);
    return_error!(snd_pcm_hw_params_current);
    return_error!(snd_pcm_hw_params_get_buffer_size);
    return_error!(snd_pcm_hw_params_get_channels);
    return_error!(snd_pcm_hw_params_get_period_size);
    return_error!(snd_pcm_hw_params_get_periods);
    return_error!(snd_pcm_hw_params_set_access);
    return_error!(snd_pcm_hw_params_set_channels);
    return_error!(snd_pcm_hw_params_set_format);
    return_error!(snd_pcm_hw_params_set_rate_near);
    return_expr!(snd_pcm_hw_params_sizeof, usize, 16 /* arbitrary */);
    return_error!(snd_pcm_open);
    return_error!(snd_pcm_prepare);
    return_error!(snd_pcm_readi);
    return_error!(snd_pcm_recover);
    return_error!(snd_pcm_resume);
    return_error!(snd_pcm_start);
    return_expr!(snd_pcm_state, c_int, 8 /* disconnected */);
    return_error!(snd_pcm_status);
    return_expr!(snd_pcm_status_get_state, c_int, 8 /* disconnected */);
    return_expr!(snd_pcm_status_sizeof, usize, 16 /* arbitrary */);
    return_error!(snd_pcm_sw_params);
    return_error!(snd_pcm_sw_params_current);
    return_error!(snd_pcm_sw_params_set_start_threshold);
    return_error!(snd_pcm_sw_params_set_stop_threshold);
    return_expr!(snd_pcm_sw_params_sizeof, usize, 16 /* arbitrary */);
    return_expr!(snd_pcm_writei, c_long, -1);
}
