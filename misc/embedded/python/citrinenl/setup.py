from setuptools import Extension, setup
PATH_TO_CITRINE = '../../../../'
setup(
    package_dir = {"": "citrine_module_nl"},
    ext_modules=[
        Extension(
            name="citrine_module_nl",
            sources=[
            "citrine_module.c",
            f"{PATH_TO_CITRINE}test.c",
            f"{PATH_TO_CITRINE}siphash.c",
            f"{PATH_TO_CITRINE}utf8.c",
            f"{PATH_TO_CITRINE}memory.c",
            f"{PATH_TO_CITRINE}util.c",
            f"{PATH_TO_CITRINE}base.c",
            f"{PATH_TO_CITRINE}collections.c",
            f"{PATH_TO_CITRINE}file.c",
            f"{PATH_TO_CITRINE}system.c",
            f"{PATH_TO_CITRINE}world.c",
            f"{PATH_TO_CITRINE}lexer.c",
            f"{PATH_TO_CITRINE}parser.c",
            f"{PATH_TO_CITRINE}walker.c",
            f"{PATH_TO_CITRINE}translator.c",
            f"{PATH_TO_CITRINE}citrine.c",
            ],
            include_dirs=[f"{PATH_TO_CITRINE}",f"{PATH_TO_CITRINE}i18n/nl",],
        ),
    ]
)