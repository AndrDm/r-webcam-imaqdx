fn main() -> std::io::Result<()> {
	println!("cargo:rustc-link-lib=lib\\niimaqdx");
	println!("cargo:rustc-link-lib=lib\\nivision");
	println!("cargo:rustc-link-lib=user32");

	Ok(())
}
