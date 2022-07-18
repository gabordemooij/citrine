from setuptools import Extension, setup
setup(
    package_dir = {"": "citrine_module_en"},
    ext_modules=[
        Extension(
            name="citrine_module_en",
            sources=[
            "citrine_module.c",
            "../../../../test.c",
            "../../../../siphash.c",
            "../../../../utf8.c",
            "../../../../memory.c",
            "../../../../util.c",
            "../../../../base.c",
            "../../../../collections.c",
            "../../../../file.c",
            "../../../../system.c",
            "../../../../world.c",
            "../../../../lexer.c",
            "../../../../parser.c",
            "../../../../walker.c",
            "../../../../translator.c",
            "../../../../citrine.c",
            ],
            include_dirs=["../../../../","../../../..//i18n/en",],
        ),
    ]
)