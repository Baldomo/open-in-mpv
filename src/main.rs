use open_in_mpv::MpvOption;
use std::{collections::HashMap, env, process::{Command, exit, Stdio}};
use url::Url;

fn main() {
    // mpv:///open?url=XXXXXXX&full_screen=1&pip=1&enqueue=0
    let raw_url: String = env::args_os().nth(1).unwrap().into_string().unwrap();
    let parsed_url: Url = match Url::parse(&raw_url) {
        Err(why) => panic!("{:?}", why),
        Ok(parsed) => parsed,
    };

    if parsed_url.scheme() != "mpv" {
        println!("Unsupported protocol: {}", parsed_url.scheme());
        exit(1);
    }

    if parsed_url.path() != "/open" {
        println!("Unsupported method: {}", parsed_url.path());
        exit(1);
    }

    let query: HashMap<String, String> = parsed_url.query_pairs().into_owned().collect();

    let mo = MpvOption::new(
        query.get("url").unwrap(),
        query.get("full_screen").unwrap(),
        query.get("pip").unwrap(),
        query.get("enqueue").unwrap(),
    );

    Command::new("mpv")
        .args(mo.build_args())
        .stdout(Stdio::null())
        .spawn()
        .expect("failed to open mpv");
}
