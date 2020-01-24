use std::vec::Vec;
use percent_encoding::percent_decode;

#[derive(Debug)]
pub struct MpvOption {
    url: String,
    fullscreen: bool,
    pip: bool,
    enqueue: bool,
}

impl MpvOption {
    /// Builds a MpvOption from the supplied arguments.
    ///
    /// # Example
    /// ```
    /// use open_in_mpv::MpvOption;
    /// let mo: MpvOption = MpvOption::new(
    ///     "url",
    ///     "1",
    ///     "0",
    ///     "0",
    /// );
    ///
    /// print!("{:?}", mo);
    /// ```
    pub fn new(url: &str, fullscreen: &str, pip: &str, enqueue: &str) -> Self {
        Self {
            url: percent_decode(url.as_bytes())
                .decode_utf8()
                .unwrap()
                .as_ref()
                .to_owned(),

            fullscreen: match fullscreen {
                "1" => true,
                "0" => false,
                _ => false,
            },

            pip: match pip {
                "1" => true,
                "0" => false,
                _ => false,
            },

            enqueue: match enqueue {
                "1" => true,
                "0" => false,
                _ => false,
            },
        }
    }


    /// Returns a Vec containing the argument list for a new mpv process
    ///
    /// Each parameter of MpvOption is bound to a set of defaults, e.g. `--ontop --no-border` etc.
    ///
    /// # Example
    /// ```
    /// use open_in_mpv::MpvOption;
    /// let mo: MpvOption = MpvOption::new(
    ///     "url",
    ///     "1",
    ///     "0",
    ///     "0",
    /// );
    ///
    /// print!("{:?}", mo.build_args());
    /// ```
    pub fn build_args(self) -> Vec<String> {
        let mut ret: Vec<String> = Vec::new();
        if self.fullscreen {
            ret.push("--fs".to_owned());
        }

        if self.pip {
            ret.push("--ontop".to_owned());
            ret.push("--no-border".to_owned());
            ret.push("--autofit=384x216".to_owned());
            ret.push("--geometry=98%:98%".to_owned());
        }

        if self.enqueue {
            // TODO: figure this out
        }

        ret.push(self.url.to_owned());

        return ret;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_build_option() {
        let encoded_url: &str = "https%3A%2F%2Fst3x_plus.cdnfile.info%2Fuser592%2F5543369133e06eedecc4907bfcd0fd45%2FEP.1.360p.mp4%3Ftoken%3DZDpvjlchVpTP0yb_5AsaEw%26expires%3D1562085204%26title%3D(360P%2520-%2520mp4)%2520Terminator%2B4%253A%2BSalvation%2BHD-720p";
        let expected_url: String = String::from("https://st3x_plus.cdnfile.info/user592/5543369133e06eedecc4907bfcd0fd45/EP.1.360p.mp4?token=ZDpvjlchVpTP0yb_5AsaEw&expires=1562085204&title=(360P%20-%20mp4)%20Terminator+4%3A+Salvation+HD-720p");

        let mo: MpvOption = MpvOption::new(
            encoded_url, 
            "1",
            "0",
            "1",
        );

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

        let args: Vec<String> = mo.build_args();
        assert_eq!(args[0], "--fs".to_owned());
        assert_eq!(args[1], "invalid_url_but_who_cares".to_owned());
    }
}
