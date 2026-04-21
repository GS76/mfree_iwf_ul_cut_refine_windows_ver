# Session Summary Template

## Session Metadata

- Date:
- Time (start–end):
- Participants:
- Facilitator:
- Repository/Branch:
- Session objectives:
- Links (PRs/issues/docs):

## Pre-Session (Before Starting)

### Context

- What problem are we trying to solve?
- What is the current state (latest known working commit/run)?
- What constraints apply (deadlines, tooling, environment, CI gates)?

### Repository Snapshot

- Branch:
- Base branch (if targeting a PR):
- HEAD commit:
- Working tree state:
  - `git status --porcelain=v1 --branch` output:
    - 
- Untracked (not ignored):
  - `git ls-files --others --exclude-standard` output:
    - 
- Ignored/generated (high-level):
  - `git ls-files --others -i --exclude-standard | Measure-Object -Line` output:
    - 

### Agenda

- Item 1:
- Item 2:
- Item 3:

### Assumptions and Risks

- Assumptions:
  - 
- Risks:
  - 
- Dependencies:
  - 

### Pre-Session Checklist

- Working tree clean or changes accounted for (`git status`).
- Reproduction steps identified (commands + inputs).
- Validation plan defined (tests/CI/outputs).
- Evidence collection plan defined (what logs/paths will be captured and where they will be stored).

## In-Session Notes

### Key Discussion Points

- 

### Observations / Evidence

- Logs/screenshots/results:
  - 
- Reproduction details:
  - 

### Decisions Made

- Decision:
  - Rationale:
  - Alternatives considered:
  - Impacted files/modules:

## Post-Session (After Ending)

### Work Completed

- Implemented:
  - 
- Changed:
  - 
- Removed:
  - 

### Version Control Updates

- Commits created (hash + message):
  - 
- Branch pushed:
  - `git push` result/notes:
    - 
- PR status:
  - Link:
  - Reviewers requested:
  - Approvals required/received:

### Action Items (Assigned)

- [ ] Action item:
  - Owner:
  - Due (target):
  - Tracking link (issue/PR):

### Next Steps (Unassigned / Upcoming)

- 

### Validation and Quality Gates

- Local checks run:
  - Command:
  - Result:
- CI checks:
  - Workflow:
  - Result:
- Runtime validation (if applicable):
  - Scenario/config:
  - Output artifacts:

### Incident / Debugging Artifacts (If Applicable)

- Logs captured:
  - Path(s):
  - Source (local run, CI run URL, issue link):
- Failure mapping artifacts (if generated):
  - CSV/MD path(s):
  - Generation command(s):

### Open Questions / Follow-Ups

- 

### Session Outcome

- Outcome summary (1–3 bullets):
  - 
