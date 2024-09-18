
# `DKCustomFormFilter`

Some widgets, like `DKFormPicker` and `DKFormListPane`, can be used to select one or more forms. You can apply a `DKCustomFormFilter` to these widgets to limit the forms that the user is allowed to select. Your custom filter can use any arbitrary criteria to limit the forms.

(That said: for custom filters attached to form pickers, you may wish to avoid fully loading the stub, as pickers are built to handle very large numbers of forms at a time. Loading every such stub may be slow.)