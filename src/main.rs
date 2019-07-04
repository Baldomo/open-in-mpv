use std::{collections::HashMap, env, process::{Command, exit, Stdio}, vec::Vec};
use url::{percent_encoding::percent_decode, Url};

#[derive(Debug)]
struct MpvOption {
    url: String,
    fullscreen: bool,
    pip: bool,
    enqueue: bool,
}

fn build_option(
    url: Option<&String>,
    fullscreen: Option<&String>,
    pip: Option<&String>,
    enqueue: Option<&String>,
) -> MpvOption {
    MpvOption {
        url: match url {
            None => String::new(),
            Some(s) => percent_decode(s.as_bytes())
                .decode_utf8()
                .unwrap()
                .as_ref()
                .to_owned(),
        },
        fullscreen: match fullscreen {
            None => false,
            Some(s) => match s.to_owned().as_ref() {
                "1" => true,
                "0" => false,
                _ => false,
            },
        },
        pip: match pip {
            None => false,
            Some(s) => match s.to_owned().as_ref() {
                "1" => true,
                "0" => false,
                _ => false,
            },
        },
        enqueue: match enqueue {
            None => false,
            Some(s) => match s.to_owned().as_ref() {
                "1" => true,
                "0" => false,
                _ => false,
            },
        },
    }
}

fn build_args(mo: MpvOption) -> Vec<String> {
    let mut ret: Vec<String> = Vec::new();
    if mo.fullscreen {
        ret.push("--fs".to_owned());
    }

    if mo.pip {
        ret.push("--ontop".to_owned());
        ret.push("--no-border".to_owned());
        ret.push("--autofit=384x216".to_owned());
        ret.push("--geometry=98%:98%".to_owned());
    }

    if mo.enqueue {
        // TODO: figure this out
    }

    ret.push(mo.url);

    return ret;
}

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

    let mo = build_option(
        query.get("url"),
        query.get("full_screen"),
        query.get("pip"),
        query.get("enqueue"),
    );

    Command::new("mpv")
        .args(build_args(mo))
        .stdout(Stdio::null())
        .spawn()
        .expect("failed to open mpv");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_build_option() {
        let one: String = String::from("1");
        let zero: String = String::from("0");
        let encoded_url: String = String::from("https%3A%2F%2Fst3x_plus.cdnfile.info%2Fuser592%2F5543369133e06eedecc4907bfcd0fd45%2FEP.1.360p.mp4%3Ftoken%3DZDpvjlchVpTP0yb_5AsaEw%26expires%3D1562085204%26title%3D(360P%2520-%2520mp4)%2520Terminator%2B4%253A%2BSalvation%2BHD-720p");

        let url_option: Option<&String> = Some(&encoded_url);
        let fullscreen_option: Option<&String> = Some(&one);
        let pip_option: Option<&String> = Some(&zero);
        let enqueue_option: Option<&String> = Some(&one);

        let expected_url: String = String::from("https://st3x_plus.cdnfile.info/user592/5543369133e06eedecc4907bfcd0fd45/EP.1.360p.mp4?token=ZDpvjlchVpTP0yb_5AsaEw&expires=1562085204&title=(360P%20-%20mp4)%20Terminator+4%3A+Salvation+HD-720p");

        let mo: MpvOption = build_option(url_option, fullscreen_option, pip_option, enqueue_option);

        assert_eq!(mo.url, expected_url);
        assert_eq!(mo.fullscreen, true);
        assert_eq!(mo.pip, false);
        assert_eq!(mo.enqueue, true);
    }

    #[test]
    fn test_build_args() {
        let mo: MpvOption = MpvOption {
            url: String::from("invalid_url_but_who_cares"),
            fullscreen: true,
            pip: false,
            enqueue: true,
        };

        let args: Vec<String> = build_args(mo);
        assert_eq!(args[0], "--fs".to_owned());
        assert_eq!(args[1], "invalid_url_but_who_cares".to_owned());
    }
}
