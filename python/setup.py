import setuptools
setuptools.setup(
    name="flasher",
    version="3.3.0",
    author="ExpressLRS Team",
    author_email="",
    description="ExpressLRS Backpack Binary Installer",
    long_description='ExpressLRS binary configurator and flasher tool all-in-one',
    long_description_content_type="text/markdown",
    url="https://github.com/ExpressLRS/Backpack",
    packages=['.'] + setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "console_scripts": ["flash=binary_configurator:main"],
    },
    install_requires=['pyserial'],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
)
