# -*- coding: utf-8 -*-
import os
import sys
import warnings
from datetime import date

from sphinx_scylladb_theme.utils import multiversion_regex_builder
from recommonmark.transform import AutoStructify

sys.path.insert(0, os.path.abspath(".."))

# -- Global variables

# Build documentation for the following tags and branches
TAGS = []
BRANCHES = ["master", "branch-5.1"]
# Set the latest version.
LATEST_VERSION = "branch-5.1"
# Set which versions are not released yet.
UNSTABLE_VERSIONS = ["master"]
# Set which versions are deprecated
DEPRECATED_VERSIONS = [""]

# -- General configuration ------------------------------------------------

# Add any Sphinx extension module names here, as strings.
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.todo",
    "sphinx.ext.mathjax",
    "sphinx.ext.githubpages",
    "sphinx.ext.extlinks",
    "sphinx_sitemap",
    "sphinx_scylladb_theme",
    "sphinx_multiversion",  # optional
    "recommonmark",  # optional
]

# The suffix(es) of source filenames.
source_suffix = [".rst", ".md"]

# The master toctree document.
master_doc = "index"

# General information about the project.
project = "ScyllaDB Open Source"
copyright = str(date.today().year) + ", ScyllaDB. All rights reserved."
author = u"ScyllaDB Project Contributors"

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'lib', 'lib64','**/_common/*', 'README.md', 'index.md', '.git', '.github', '_utils', 'rst_include', 'venv', 'dev']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = "sphinx"

# List of substitutions
rst_prolog = """
.. |mon_root| replace::  `Scylla Monitoring Stack <https://monitoring.docs.scylladb.com>`__
.. |cql-version| replace:: 3.3.1
"""
# -- Options for not found extension ---------------------------------------

# Template used to render the 404.html generated by this extension.
notfound_template = "404.html"

# Prefix added to all the URLs generated in the 404 page.
notfound_urls_prefix = ""

# -- Options for sitemap extension ---------------------------------------

sitemap_url_scheme = "stable/{link}"

# -- Options for multiversion extension ----------------------------------

# Whitelist pattern for tags
smv_tag_whitelist = multiversion_regex_builder(TAGS)
# Whitelist pattern for branches
smv_branch_whitelist = multiversion_regex_builder(BRANCHES)
# Defines which version is considered to be the latest stable version.
smv_latest_version = LATEST_VERSION
# Defines the new name for the latest version.
smv_rename_latest_version = "stable"
# Whitelist pattern for remotes (set to None to use local branches only)
smv_remote_whitelist = r"^origin$"
# Pattern for released versions
smv_released_pattern = r"^tags/.*$"
# Format for versioned output directories inside the build directory
smv_outputdir_format = "{ref.name}"

# -- Options for HTML output ----------------------------------------------

# The theme to use for pages.
html_theme = "sphinx_scylladb_theme"

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for the theme, see the
# documentation.
html_theme_options = {
    "conf_py_path": "docs/",
    "hide_edit_this_page_button": "false",
    "github_issues_repository": "scylladb/scylladb",
    "github_repository": "scylladb/scylladb",
    "versions_unstable": UNSTABLE_VERSIONS,
    "versions_deprecated": DEPRECATED_VERSIONS,
    'banner_button_text': 'Register for Free',
    'banner_button_url': 'https://www.scylladb.com/scylladb-summit-2023',
    'banner_title_text': 'ScyllaDB Summit 2023 Online Conference | February 15-16',
    'hide_banner': 'false',
    "collapse_navigation": 'true',
}

# Last updated format
html_last_updated_fmt = "%d %b %Y"

# Custom sidebar templates, maps document names to template names.
html_sidebars = {"**": ["side-nav.html"]}

# Output file base name for HTML help builder.
htmlhelp_basename = "ScyllaDocumentationdoc"

# URL which points to the root of the HTML documentation.
html_baseurl = 'https://docs.scylladb.com'

# Dictionary of values to pass into the template engine’s context for all pages
html_context = {"html_baseurl": html_baseurl}


# -- Initialize Sphinx ----------------------------------------------
def setup(sphinx):
    warnings.filterwarnings(
        action="ignore",
        category=UserWarning,
        message=r".*Container node skipped.*",
    )
    sphinx.add_config_value('recommonmark_config', {
        'enable_eval_rst': True,
    }, True)
    sphinx.add_transform(AutoStructify)
