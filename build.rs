extern crate bindgen;
extern crate cc;

use std::{env, fs, path::PathBuf, process::Command};

const SDK_PATH: &str = "vendor/Magewell_Capture_SDK_Linux_3.3.1.1313";

fn main() {
    #[cfg(not(target_os = "linux"))]
    panic!("only linux is supported at the moment");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    let target_arch = env::var("CARGO_CFG_TARGET_ARCH").expect("CARGO_CFG_TARGET_ARCH must be set");

    let vendor_lib_path = format!(
        "{}/{}/Lib/{}",
        env!("CARGO_MANIFEST_DIR"),
        SDK_PATH,
        match target_arch.as_str() {
            "x86_64" => "x64",
            "aarch64" => "arm64",
            _ => panic!("unsupported target arch"),
        }
    );

    let lib_path = out_path.join("lib");
    fs::create_dir_all(&lib_path).unwrap();
    fs::copy(
        format!("{}/libMWCapture.a", vendor_lib_path),
        lib_path.join("libMWCapture.a"),
    )
    .unwrap();

    println!("cargo:rustc-link-search={}", lib_path.display());
    println!("cargo:rustc-link-lib=static=MWCapture");

    println!("cargo:rustc-link-lib=stdc++");

    if env::var("CARGO_FEATURE_NO_DEPS").is_ok() {
        // Strip libusb from the build so we can stub it out.
        Command::new("ar")
            .args([
                "d",
                lib_path.join("libMWCapture.a").to_str().unwrap(),
                "libusb_1_0_la-linux_udev.o",
                "libusb_1_0_la-linux_usbfs.o",
                "libusb_1_0_la-core.o",
                "libusb_1_0_la-io.o",
                "libusb_1_0_la-hotplug.o",
                "libusb_1_0_la-descriptor.o",
                "libusb_1_0_la-sync.o",
            ])
            .status()
            .expect("failed to remove libudev.a from libMWCapture.a");
    } else {
        println!("cargo:rustc-link-lib=v4l2");
        println!("cargo:rustc-link-lib=asound");
        println!("cargo:rustc-link-lib=udev");
    }

    cc::Build::new()
        .include(format!("{}/Include", SDK_PATH))
        .file("src/lib.cpp")
        .compile("magewell_capture_rs");

    let bindings = bindgen::Builder::default()
        .clang_arg(format!("-I{}/Include", SDK_PATH))
        .header("src/lib.hpp")
        .allowlist_function("MW.+")
        .allowlist_var("MW.+")
        .generate()
        .expect("unable to generate bindings");

    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("unable to write bindings");
}
