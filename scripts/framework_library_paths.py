from os.path import join

Import("env")

framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
env.Append(
    CPPPATH=[
        join(framework_dir, "libraries", "WiFi", "src"),
        join(framework_dir, "libraries", "WiFiClientSecure", "src"),
    ]
)
