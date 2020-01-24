use open_in_mpv::MpvOption;
use std::{env, fs};
use std::collections::HashMap;
use std::process::{Command, exit, Stdio};
use url::Url;

fn main() {
    // mpv:///open?url=XXXXXXX&full_screen=1&pip=1&enqueue=0
    let raw_arg: String = env::args_os().nth(1).unwrap().into_string().unwrap();

    // If first argument is "install-protocol", install the protocol file
    if raw_arg == "install-protocol" {
        install_protocol();
        exit(0);
    }

    // Parse the String in raw_arg
    let parsed_url: Url = match Url::parse(&raw_arg) {
        Ok(parsed) => parsed,
        Err(e) => {
            println!("Error parsing url: {}", raw_arg);
            println!("{}", e);
            exit(1);
        },
    };

    // Check if url has the correct protocol format
    if parsed_url.scheme() != "mpv" {
        println!("Unsupported protocol: {}", parsed_url.scheme());
        exit(1);
    }

    // Check for supported methods of operation (just `open` for now)
    if parsed_url.path() != "/open" {
        println!("Unsupported method: {}", parsed_url.path());
        exit(1);
    }

    // Convert query into a HashMap
    let query: HashMap<String, String> = parsed_url.query_pairs().into_owned().collect();

    if !query.contains_key("url") {
        println!("Url cannot be empty!");
        exit(1);
    }

    // Build a new MpvOption object
    // Note: it's now safe to unwrap all the Option's with fallbacks
    let mo = MpvOption::new(
        query.get("url").unwrap(),
        query.get("full_screen")
            .as_deref()
            .unwrap_or(&"0".to_owned()),
        query.get("pip")
            .as_deref()
            .unwrap_or(&"0".to_owned()),
        query.get("enqueue")
            .as_deref()
            .unwrap_or(&"0".to_owned()),
    );

    // Spawn a new mpv process
    Command::new("mpv")
        .args(mo.build_args())
        .stdout(Stdio::null())
        .spawn()
        .expect("failed to open mpv");
    
    exit(0);
}

// Installs the protocol file in "$HOME/.local/share/applications/open-in-mpv.desktop"
fn install_protocol() {
    const PROTOCOL_FILE: &str = "[Desktop Entry]
Name=open-in-mpv
Exec=open-in-mpv %u
Type=Application
Terminal=false
NoDisplay=true
MimeType=x-scheme-handler/mpv
";

    // Fetch $HOME
    let mut file_path: String = String::new();
    match env::var("HOME") {
        Ok(val) => file_path.push_str(&val),
        Err(e) => {
            println!("couldn't interpret HOME: {}", e);
            return;
        },
    }

    // Refine file path
    file_path.push_str("/.local/share/applications/open-in-mpv.desktop");
    
    fs::write(file_path, PROTOCOL_FILE).expect("Error writing file!");
}
