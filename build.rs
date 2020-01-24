use std::fs::{read, rename, File};
use std::io::Write;
use std::process::Command;

use byteorder::{LittleEndian, WriteBytesExt};

use openssl::error::ErrorStack;
use openssl::hash::MessageDigest;
use openssl::pkey::{PKey, Private};
use openssl::sign::Signer;

type Buffer = Vec<u8>;

// TODO: signing for the Chrome extension

fn main() {
    zip_dir("Chrome", "Chrome/ext.zip");
    package_crx("./Chrome/ext.zip", "cert.pem");
    
    zip_dir("Firefox", "Firefox/ext.zip");
    package_xpi("Firefox/ext.zip");
}

fn zip_dir(path: &str, out: &str) {
    Command::new("zip")
        .arg("-FS")
        .arg(out)
        .arg(path)
        .output()
        .unwrap();
}

fn package_xpi(source: &str) {
    let trimmed: &str = source.trim_end_matches(".zip");
    rename(source, format!("{}.xpi", trimmed)).unwrap();
}

fn package_crx(source: &str, pem: &str) {
    // read input file and store them into buffers
    let mut source_buff = read(source).unwrap();

    let user_key = load_key_from_file(pem).unwrap();

    let mut public_key_content: Vec<u8> = user_key.public_key_to_der().unwrap();

    let mut signer = Signer::new(MessageDigest::sha1(), &user_key).unwrap();

    signer.update(&source_buff).unwrap();

    let mut signature: Vec<u8> = signer.sign_to_vec().unwrap();

    let mut pub_key_len: Vec<u8> = vec![];
    pub_key_len
        .write_u32::<LittleEndian>(public_key_content.len() as u32)
        .unwrap();

    let mut sig_len: Vec<u8> = vec![];
    sig_len
        .write_u32::<LittleEndian>(signature.len() as u32)
        .unwrap();

    let trimmed_filename: &str = source.trim_end_matches(".zip");
    let mut package_file: File = File::create(format!("{}.crx", trimmed_filename)).unwrap();
    let mut package_buffer: Vec<u8> = Vec::new();

    let mut magic_bytes: Buffer = vec![0x43, 0x72, 0x32, 0x34];
    let mut version: Buffer = vec![0x02, 0x00, 0x00, 0x00];

    package_buffer.append(&mut magic_bytes);
    package_buffer.append(&mut version);
    package_buffer.append(&mut pub_key_len);
    package_buffer.append(&mut sig_len);
    package_buffer.append(&mut public_key_content);
    package_buffer.append(&mut signature);
    package_buffer.append(&mut source_buff);

    package_file.write_all(&package_buffer).unwrap();
}

fn load_key_from_file(path: &str) -> Result<PKey<Private>, ErrorStack> {
    // Read the private key from the file
    let pem_buff = read(path).unwrap();

    PKey::private_key_from_pem(&pem_buff)
}
