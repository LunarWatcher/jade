import { View, makeBook } from './foliate-js/view.js'
import { createTOCView } from './foliate-js/ui/tree.js'
import { Overlayer } from './foliate-js/overlayer.js'

const getCSS = ({ spacing, justify, hyphenate }) => `
    @namespace epub "http://www.idpf.org/2007/ops";

    p, li, blockquote, dd {
        line-height: ${spacing};
        text-align: ${justify ? 'justify' : 'start'};
        -webkit-hyphens: ${hyphenate ? 'auto' : 'manual'};
        hyphens: ${hyphenate ? 'auto' : 'manual'};
        -webkit-hyphenate-limit-before: 3;
        -webkit-hyphenate-limit-after: 2;
        -webkit-hyphenate-limit-lines: 2;
        hanging-punctuation: allow-end last;
        widows: 2;
    }

    [align="left"] { text-align: left; }
    [align="right"] { text-align: right; }
    [align="center"] { text-align: center; }
    [align="justify"] { text-align: justify; }

    pre {
        white-space: pre-wrap !important;
    }
    aside[epub|type~="endnote"],
    aside[epub|type~="footnote"],
    aside[epub|type~="note"],
    aside[epub|type~="rearnote"] {
        display: none;
    }
`

const locales = 'en'
const percentFormat = new Intl.NumberFormat(locales, { style: 'percent' })
const listFormat = new Intl.ListFormat(locales, { style: 'short', type: 'conjunction' })

const formatLanguageMap = x => {
    if (!x) return ''
    if (typeof x === 'string') return x
    const keys = Object.keys(x)
    return x[keys[0]]
}

class Reader {
    #tocView
    style = {
        spacing: 1.4,
        justify: true,
        hyphenate: true,
    }

    expandIcon = "⛶";
    shrinkIcon = "";

    rootElem = /** @type {HTMLElement} */ (document.getElementById("reader-root"));

    sidebar = /** @type {HTMLElement} */ (document.getElementById("reader-sidebar"));
    sidebarButton = /** @type {HTMLElement} */ (document.getElementById("sidebar-button"));
    sidebarCover = /** @type {HTMLImageElement} */ (document.getElementById("sidebar-cover"));
    sidebarHeader = /** @type {HTMLElement} */ (document.getElementById("sidebar-header"));
    sidebarToc = /** @type {HTMLElement} */ (document.getElementById("sidebar-toc"));
    noTocPlaceholder = /** @type {HTMLElement} */ (document.getElementById("no-toc"));
    closeSidebarBtn = /** @type {HTMLElement} */ (document.getElementById("close-sidebar"));

    readerNavbar = /** @type {HTMLElement} */ (document.getElementById("reader-navbar"));

    leftButton = /** @type {HTMLElement} */ (document.getElementById("left-button"));
    rightButton = /** @type {HTMLElement} */ (document.getElementById("right-button"));
    progressSlider = /** @type {HTMLInputElement} */ (document.getElementById("progress-slider"));
    sliderTickmarks = /** @type {HTMLDataListElement} */ (document.getElementById("tick-marks"));
    
    dimmingOverlay = /** @type {HTMLElement} */ (document.getElementById("reader-overlay"));

    menuContainer = /** @type {HTMLElement} */ (document.getElementById("menu-container"));
    menuButton = /** @type {HTMLButtonElement} */ (document.getElementById("menu-button"));
    closeSettingsButton = /** @type {HTMLButtonElement} */ (document.getElementById("close-settings"));
    flowModeContainer = /** @type {HTMLSelectElement} */ (document.getElementById("setting-flow-mode"));
    spreadModeContainer = /** @type {HTMLSelectElement} */ (document.getElementById("setting-spread-mode"));

    helpContainer = /** @type {HTMLElement} */ (document.getElementById("help-container"));
    helpButton = /** @type {HTMLButtonElement} */ (document.getElementById("help-button"));
    closeHelpContainer = /** @type {HTMLButtonElement} */ (document.getElementById("close-help"));

    fullscreenBtn = /** @type {HTMLButtonElement} */ (document.getElementById("fullscreen-button"));

    currElem = /** @type {HTMLElement | null}*/ (null);

    constructor() {

        this.sidebarButton.addEventListener('click', () => {
            this.dimmingOverlay.classList.add('show')
            this.sidebar.classList.add('show')
        })
        this.closeSidebarBtn.addEventListener("click", () => {
            this.closeSidebar();
        });
        this.dimmingOverlay.addEventListener('click', () => this.closeSidebar())


        document.addEventListener("click", ev => {
            if (ev.target != null 
                && this.currElem != null
                && ev.target != this.currElem
                && !this.currElem.contains(ev.target)
            ) {
                this.setFocus(null);
            }
        });

        this.initFSButton();

        this.registerPopupButton(this.helpButton, this.helpContainer);
        this.registerPopupCloseButton(this.closeHelpContainer);

        this.registerPopupButton(this.menuButton, this.menuContainer);
        this.registerPopupCloseButton(this.closeSettingsButton);
    }

    closeSidebar() {
        this.dimmingOverlay.classList.remove('show')
        this.sidebar.classList.remove('show')
    }

    setFocus(/** @type {HTMLElement | null} */container) {

        if (container == null) {
            if (this.currElem != null) {
                this.currElem.classList.toggle('show');
                this.currElem = null;
            }
            return;
        }

        if (this.currElem === null) {
            // Option 1: no current element. Set and show
            this.currElem = container;
            this.currElem.classList.toggle('show');
        } else if (this.currElem == container) {
            // Option 2: Current element equals the new element. This is a toggle
            this.currElem.classList.toggle('show');
            this.currElem = null;
        } else if (this.currElem != container) {
            // Option 3: Current element differs from the new element. This is a switch.
            this.currElem.classList.toggle('show');
            this.currElem = container;
            this.currElem.classList.toggle('show');
        }
    }

    setupViewMode(/** @type {Boolean} */isFlexible) {
        this.rendererChangeListener(this.flowModeContainer, "flow");
        this.rendererChangeListener(this.spreadModeContainer, "spread", (val) => {
            if (!this.view.isFixedLayout) {
                if (val == "auto") {
                    this.view.renderer.setAttribute("max-column-count", "2");
                } else if (val == "both") {
                    this.view.renderer.setAttribute("max-column-count", "2");
                } else if (val == "portrait") {
                    this.view.renderer.setAttribute("max-column-count", "1");
                } else {
                    throw Error("Wtf");
                }
            }
            return val;
        });

        if (!isFlexible) {
            for (let elem of document.getElementsByClassName("setting-needsflexible")) {
                elem.disabled = true;
            }
        } else if (isFlexible) {
            for (let elem of document.getElementsByClassName("setting-needsnotflexible")) {
                elem.disabled = true;
            }

        }
    }

    rendererChangeListener(
        /** @type {HTMLSelectElement} */
        elem,
        /** @type {String} */
        property,
        /** @type {((src: String) => String) | null} */
        filter = null
    ) {
        const setAttr = () => {
            this.view?.renderer.setAttribute(
                property, 
                (() => {
                    if (filter != null) {
                        return filter(elem.value);
                    }

                    return elem.value;
                })()
            )
        };
        elem.addEventListener("change", () => {
            setAttr()
        });

        // Invoked once during setup to force handling of weird caching of form options that differ from
        // the default values set in foliate.
        // There's probably a better way to do this, but the docs are non-existent, so I'm brute-forcing
        // solutions as I go.
        setAttr();
    }

    async open(/** @type {String} */file) {
        this.view = /** @type View */ (document.getElementsByTagName('foliate-view')[0]);
        this.view.focus();


        await this.view.open(file);
        this.view.addEventListener('load', this.#onLoad.bind(this))
        this.view.addEventListener('relocate', this.#onRelocate.bind(this))

        const { book } = this.view;
        book.transformTarget?.addEventListener('data', ({ detail }) => {
            detail.data = Promise.resolve(detail.data).catch(e => {
                showDialog(`Failed to load ${detail.name}`);
                console.error(e);
                return "";
            })
        })
        this.view.renderer.setStyles?.(getCSS(this.style));
        this.view.renderer.next();

        console.log(book);
        this.setupViewMode(
            book.rendition.layout != "pre-paginated"
        );

        this.sidebarHeader.style.visibility = 'visible';
        this.readerNavbar.style.visibility = 'visible';
        this.leftButton.addEventListener('click', () => this.view.goLeft())
        this.rightButton.addEventListener('click', () => this.view.goRight())

        const slider = this.progressSlider;
        slider.dir = book.dir
        slider.addEventListener('input', e =>
            this.view.goToFraction(parseFloat(e.target.value))
        );
        for (const fraction of this.view.getSectionFractions()) {
            const option = document.createElement('option')
            option.value = fraction
            this.sliderTickmarks.append(option)
        }

        document.addEventListener('keydown', this.#handleKeydown.bind(this))

        Promise.resolve(book.getCover?.())?.then(blob =>
            blob ? this.sidebarCover.src = URL.createObjectURL(blob) : null)

        const toc = book.toc
        if (toc) {
            this.#tocView = createTOCView(toc, href => {
                this.view.goTo(href).catch(e => console.error(e))
                //this.closeSidebar()
            })
            this.noTocPlaceholder.style.visibility = "collapse";
            this.sidebarToc.append(this.#tocView.element)
        }
    }
    #handleKeydown(/** @type {KeyboardEvent} */ event) {
        const k = event.key;
        if (k === "ArrowLeft" || k === "h") {
            this.view?.goLeft();
        } else if (k === "ArrowRight" || k === "l") {
            this.view?.goRight();
        } else if (k == "Escape" || k == "e") {
            this.closeSidebar();
            this.setFocus(null);
        }
    }
    #onLoad({ detail: { doc } }) {
        doc.addEventListener('keydown', this.#handleKeydown.bind(this))
    }
    #onRelocate({ detail }) {
        const { fraction, location, tocItem, pageItem } = detail
        const percent = percentFormat.format(fraction)
        const loc = pageItem
            ? `Page ${pageItem.label}`
            : `Loc ${location.current}`
        this.progressSlider.style.visibility = 'visible'
        this.progressSlider.value = fraction
        this.progressSlider.title = `${percent} · ${loc}`
        if (tocItem?.href) this.#tocView?.setCurrentHref?.(tocItem.href)
    }

    registerPopupButton(/** @type {HTMLButtonElement} */ elem, /** @type {HTMLElement} */ container) {
        elem.addEventListener('click', (ev) => {
            ev.stopPropagation();
            this.setFocus(container);
        });
    }

    registerPopupCloseButton(/** @type {HTMLButtonElement} */ elem) {
        elem.addEventListener("click", () => {
            this.setFocus(null);
        });
    }

    initFSButton() {
        if (document.fullscreenEnabled) {
            this.fullscreenBtn.addEventListener("click", () => {
                if (document.fullscreenElement == null) {
                    this.rootElem.requestFullscreen();
                } else {
                    document.exitFullscreen();
                }
            });
            this.fullscreenBtn.addEventListener("fullscreenchange", () => {
                this.fullscreenBtn.innerText = (document.fullscreenElement != null) ? this.expandIcon : this.shrinkIcon;
            });
        } else {
            this.fullscreenBtn.style.visibility = "collapse";
        }
    }
}

const open = async file => {
    const reader = new Reader()
    globalThis.reader = reader
    await reader.open(file)
}

let bookID = document.getElementById("reader-root")
    .getAttribute("data-bookid");
open(`/api/books/${bookID}/download`)
    .catch(e => {
        showDialog("Failed to load the book. Download it from the details page and read it in a different reader, or try again later");
        console.error(e)
    });
